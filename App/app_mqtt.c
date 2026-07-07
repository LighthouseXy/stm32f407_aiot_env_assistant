/**
  ******************************************************************************
  * @file           : app_mqtt.c
  * @brief          : MQTT client implementation (placeholder)
  ******************************************************************************
  */

#include "app_mqtt.h"
#include "app_state.h"
#include "app_wifi.h"
#include <stdio.h>
#include <string.h>

#define APP_MQTT_BROKER_HOST       "broker.hivemq.com"
#define APP_MQTT_BROKER_PORT       1883U
#define APP_MQTT_CLIENT_ID         "stm32f407-aiot-env"
#define APP_MQTT_PACKET_BUF_SIZE   128U
#define APP_MQTT_RX_BUF_SIZE       128U
#define APP_MQTT_STEP_INTERVAL_MS  3000U

typedef enum {
    APP_MQTT_STEP_WAIT_WIFI = 0,
    APP_MQTT_STEP_TCP_CONNECT,
    APP_MQTT_STEP_MQTT_CONNECT,
    APP_MQTT_STEP_WAIT_CONNACK,
    APP_MQTT_STEP_MQTT_PUBLISH,
    APP_MQTT_STEP_DONE,
    APP_MQTT_STEP_ERROR
} AppMqttStep_t;

/* Private variables -------------------------------------------------------*/
static uint32_t last_print_tick = 0;
/* MQTT 任务栈较小，大缓冲区放到静态区，避免任务栈溢出。 */
static char mqtt_cmd_buf[96];
/* MQTT 接收缓存用于保存 ESP01S 返回的 +IPD / CONNACK 数据。 */
static uint8_t mqtt_rx_buf[APP_MQTT_RX_BUF_SIZE];
static uint8_t mqtt_packet[APP_MQTT_PACKET_BUF_SIZE];

static int AppMqtt_WriteString(uint8_t *buf,
                               uint16_t max_len,
                               uint16_t *pos,
                               const char *text)
{
    uint16_t text_len;

    if ((buf == NULL) || (pos == NULL) || (text == NULL)) {
        return -1;
    }

    text_len = (uint16_t)strlen(text);

    if ((*pos + 2U + text_len) > max_len) {
        return -1;
    }

    /* MQTT 字符串格式：2 字节长度 + 字符串内容。 */
    buf[(*pos)++] = (uint8_t)(text_len >> 8);
    buf[(*pos)++] = (uint8_t)(text_len & 0xFFU);
    memcpy(&buf[*pos], text, text_len);
    *pos += text_len;

    return 0;
}

static int AppMqtt_BuildConnectPacket(uint8_t *buf,
                                      uint16_t max_len,
                                      uint16_t *packet_len)
{
    uint16_t pos = 0U;
    uint16_t remaining_len;

    if ((buf == NULL) || (packet_len == NULL)) {
        return -1;
    }

    /*
     * MQTT CONNECT 固定报文：
     * Fixed header:
     *   0x10 = CONNECT
     *   remaining length
     * Variable header:
     *   Protocol Name = "MQTT"
     *   Protocol Level = 4
     *   Connect Flags = 0x02 clean session
     *   Keep Alive = 60 seconds
     * Payload:
     *   Client ID
     */
    remaining_len = (uint16_t)(10U + 2U + strlen(APP_MQTT_CLIENT_ID));

    if ((remaining_len >= 128U) || ((2U + remaining_len) > max_len)) {
        return -1;
    }

    buf[pos++] = 0x10U;
    buf[pos++] = (uint8_t)remaining_len;

    if (AppMqtt_WriteString(buf, max_len, &pos, "MQTT") != 0) {
        return -1;
    }

    buf[pos++] = 0x04U;  /* MQTT 3.1.1 */
    buf[pos++] = 0x02U;  /* Clean Session */
    buf[pos++] = 0x00U;
    buf[pos++] = 0x3CU;  /* Keep Alive = 60s */

    if (AppMqtt_WriteString(buf, max_len, &pos, APP_MQTT_CLIENT_ID) != 0) {
        return -1;
    }

    *packet_len = pos;
    return 0;
}

static int AppMqtt_BuildPublishPacket(uint8_t *buf,
                                      uint16_t max_len,
                                      uint16_t *packet_len,
                                      const AppState_t *state)
{
    const char *topic = "stm32f407/env";
    char payload[96];
    uint16_t pos = 0U;
    uint16_t topic_len;
    uint16_t payload_len;
    uint16_t remaining_len;

    if ((buf == NULL) || (packet_len == NULL) || (state == NULL)) {
      return -1;
    }

    int temp_x10 = (int)(state->temperature * 10.0f);
    int hum_x10 = (int)(state->humidity * 10.0f);

    /* 使用 JSON 文本作为 MQTT Payload，方便 MQTT 客户端直接查看数据。 */
    snprintf(payload, sizeof(payload),
             "{\"temp\":%d.%d,\"hum\":%d.%d,\"light\":%d,\"mode\":\"%s\"}",
             temp_x10 / 10,
             temp_x10 % 10,
             hum_x10 / 10,
             hum_x10 % 10,
             state->light_percent,
             AppState_ModeToString(state->mode));
    topic_len = (uint16_t)strlen(topic);
    payload_len = (uint16_t)strlen(payload);

    /* QoS0 PUBLISH 报文：topic length + topic + payload。 */
    remaining_len = (uint16_t)(2U + topic_len + payload_len);

    if ((remaining_len >= 128U) || ((2U + remaining_len) > max_len)) {
        return -1;
    }

    buf[pos++] = 0x30U;                 /* MQTT PUBLISH, QoS0 */
    buf[pos++] = (uint8_t)remaining_len;

    buf[pos++] = (uint8_t)(topic_len >> 8);
    buf[pos++] = (uint8_t)(topic_len & 0xFFU);
    memcpy(&buf[pos], topic, topic_len);
    pos += topic_len;

    memcpy(&buf[pos], payload, payload_len);
    pos += payload_len;

    *packet_len = pos;
    return 0;
}

static int AppMqtt_HasConnack(const uint8_t *buf, uint16_t len)
{
    if (buf == NULL) {
        return 0;
    }

    /* MQTT CONNACK 成功报文为 20 02 00 00。 */
    for (uint16_t i = 0U; (i + 3U) < len; i++) {
        if ((buf[i] == 0x20U) &&
            (buf[i + 1U] == 0x02U) &&
            (buf[i + 2U] == 0x00U) &&
            (buf[i + 3U] == 0x00U)) {
            return 1;
        }
    }

    return 0;
}

/* Exported functions ------------------------------------------------------*/
void AppMqtt_Init(void)
{
    AppState_t *state = AppState_Get();
    state->mqtt_status = APP_MQTT_OFF;
    last_print_tick = 0;
}

void AppMqtt_Process(void)
{
    static AppMqttStep_t mqtt_step = APP_MQTT_STEP_WAIT_WIFI;
    static uint32_t last_step_tick = 0U;

    AppState_t *state = AppState_Get();
    uint32_t now = state->tick_ms;
    uint16_t packet_len = 0U;

    if ((now - last_step_tick) < APP_MQTT_STEP_INTERVAL_MS) {
        return;
    }
    last_step_tick = now;

    switch (mqtt_step) {
    case APP_MQTT_STEP_WAIT_WIFI:
        if (state->wifi_status == APP_WIFI_CONNECTED) {
            state->mqtt_status = APP_MQTT_CONNECTING;
            mqtt_step = APP_MQTT_STEP_TCP_CONNECT;
            printf("[mqtt] wifi ready, start tcp connect\r\n");
        }
        break;

    case APP_MQTT_STEP_TCP_CONNECT:
      /*
       * WiFi 刚拿到 IP 后，ESP01S 可能还在处理联网状态。
       * 先用 AT 探测模块是否可响应，失败则等待下一轮重试。
       */
      if (AppWifi_SendCommand("AT", "OK", 3000U) != 0) {
        printf("[mqtt] esp not ready, retry tcp connect later\r\n");
        state->mqtt_status = APP_MQTT_CONNECTING;
        break;
      }

      /*
       * CIPMUX 是 TCP 连接前的本地配置命令。
       * 偶发超时不直接判定 MQTT 失败，下一轮继续重试。
       */
      if (AppWifi_SendCommand("AT+CIPMUX=0", "OK", 5000U) != 0) {
        printf("[mqtt] CIPMUX failed, retry later\r\n");
        state->mqtt_status = APP_MQTT_CONNECTING;
        break;
      }

      snprintf(mqtt_cmd_buf, sizeof(mqtt_cmd_buf),
               "AT+CIPSTART=\"TCP\",\"%s\",%u", APP_MQTT_BROKER_HOST,
               (unsigned int)APP_MQTT_BROKER_PORT);

      if (AppWifi_SendCommand(mqtt_cmd_buf, "OK", 15000U) == 0) {
        printf("[mqtt] tcp connect OK\r\n");
        mqtt_step = APP_MQTT_STEP_MQTT_CONNECT;
      } else {
        printf("[mqtt] tcp connect failed\r\n");
        state->mqtt_status = APP_MQTT_ERROR;
        mqtt_step = APP_MQTT_STEP_ERROR;
      }
      break;

    case APP_MQTT_STEP_MQTT_CONNECT:
      if (AppMqtt_BuildConnectPacket(mqtt_packet, sizeof(mqtt_packet),
                                     &packet_len) != 0) {
        printf("[mqtt] build CONNECT packet failed\r\n");
        state->mqtt_status = APP_MQTT_ERROR;
        mqtt_step = APP_MQTT_STEP_ERROR;
        break;
      }

      if (AppWifi_SendTcpPayload(mqtt_packet, packet_len, mqtt_rx_buf,
                                 sizeof(mqtt_rx_buf), &packet_len,
                                 5000U) == 0) {
        printf("[mqtt] CONNECT packet SEND OK\r\n");

        if (AppMqtt_HasConnack(mqtt_rx_buf, packet_len) != 0) {
          printf("[mqtt] CONNACK OK\r\n");
          state->mqtt_status = APP_MQTT_CONNECTED;
          /* CONNACK 后留一轮调度间隔，避免 +IPD 二进制尾巴影响下一条 CIPSEND。
           */
          mqtt_step = APP_MQTT_STEP_MQTT_PUBLISH;
          last_step_tick = now;
        } else {
          /*
           * 有时本轮只收到 +IPD,4:，真正的 CONNACK 4 字节会紧跟在后面。
           * 不立即判失败，进入补收状态再读一次。
           */
          printf("[mqtt] CONNACK missing, wait more\r\n");
          state->mqtt_status = APP_MQTT_CONNECTING;
          mqtt_step = APP_MQTT_STEP_WAIT_CONNACK;
          last_step_tick = 0U;
        }
      } else {
        printf("[mqtt] CONNECT packet SEND failed\r\n");
        state->mqtt_status = APP_MQTT_ERROR;
        mqtt_step = APP_MQTT_STEP_ERROR;
      }
      break;

    case APP_MQTT_STEP_WAIT_CONNACK:
      if (AppWifi_ReceiveTcpData(mqtt_rx_buf, sizeof(mqtt_rx_buf), &packet_len,
                                 3000U) == 0) {
        if (AppMqtt_HasConnack(mqtt_rx_buf, packet_len) != 0) {
          printf("[mqtt] CONNACK OK\r\n");
          state->mqtt_status = APP_MQTT_CONNECTED;
          mqtt_step = APP_MQTT_STEP_DONE;
        } else {
          printf("[mqtt] CONNACK invalid\r\n");
          state->mqtt_status = APP_MQTT_ERROR;
          mqtt_step = APP_MQTT_STEP_ERROR;
        }
      } else {
        printf("[mqtt] CONNACK timeout\r\n");
        state->mqtt_status = APP_MQTT_ERROR;
        mqtt_step = APP_MQTT_STEP_ERROR;
      }
      break;

    case APP_MQTT_STEP_MQTT_PUBLISH:
      if (AppMqtt_BuildPublishPacket(mqtt_packet, sizeof(mqtt_packet),
                                     &packet_len, state) != 0) {
        printf("[mqtt] build PUBLISH packet failed\r\n");
        state->mqtt_status = APP_MQTT_ERROR;
        mqtt_step = APP_MQTT_STEP_ERROR;
        break;
      }

      if (AppWifi_SendTcpPayload(mqtt_packet, packet_len, mqtt_rx_buf,
                                 sizeof(mqtt_rx_buf), &packet_len,
                                 5000U) == 0) {
        printf("[mqtt] PUBLISH packet SEND OK\r\n");
        mqtt_step = APP_MQTT_STEP_DONE;
      } else {
        printf("[mqtt] PUBLISH packet SEND failed\r\n");
        state->mqtt_status = APP_MQTT_ERROR;
        mqtt_step = APP_MQTT_STEP_ERROR;
      }
      break;

    case APP_MQTT_STEP_DONE:
      /* 第三步只验证 MQTT CONNECT 报文已通过 TCP 发出。 */
      break;

    case APP_MQTT_STEP_ERROR:
    default:
        break;
    }
}