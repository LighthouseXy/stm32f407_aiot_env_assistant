/**
  ******************************************************************************
  * @file           : app_wifi.c
  * @brief          : WiFi module implementation (placeholder)
  ******************************************************************************
  */

#include "app_wifi.h"
#include "app_state.h"
#include <stdio.h>
#include <string.h>
#include "main.h"

/* 使用 CubeMX 生成的 USART3 句柄，USART3 连接 ESP01S */
extern UART_HandleTypeDef huart3;
/* Private variables -------------------------------------------------------*/
static uint32_t last_print_tick = 0;
/* ESP01S AT 指令测试只执行一次，避免日志反复刷屏 */
static uint8_t esp_at_test_done = 0;

#define APP_WIFI_RX_BUF_SIZE            256U
#define APP_WIFI_TX_BUF_SIZE            96U
#define APP_WIFI_AT_DRAIN_MS            50U
#define APP_WIFI_AT_SHORT_TIMEOUT_MS    2000U
#define APP_WIFI_AT_MAX_RETRY           3U
#define APP_WIFI_AT_MODE_TIMEOUT_MS     5000U
#define APP_WIFI_JOIN_TIMEOUT_MS        30000U
#define APP_WIFI_IP_TIMEOUT_MS          5000U

#define APP_WIFI_SSID                   "iPhone13mini"
#define APP_WIFI_PASSWORD               "1234567890"

typedef enum {
    APP_WIFI_AT_OK = 0,
    APP_WIFI_AT_TX_ERROR,
    APP_WIFI_AT_REPLY_ERROR,
    APP_WIFI_AT_TIMEOUT
} AppWifiAtResult_t;

typedef struct {
    char text[APP_WIFI_RX_BUF_SIZE];
    uint16_t len;
} AppWifiAtResponse_t;

typedef struct {
    const char *cmd;
    const char *expect;
    uint32_t timeout_ms;
} AppWifiAtCommand_t;

typedef enum {
    APP_WIFI_STEP_AT = 0,
    APP_WIFI_STEP_ATE0,
    APP_WIFI_STEP_CWMODE,
    APP_WIFI_STEP_CWJAP,
    APP_WIFI_STEP_CIFSR,
    APP_WIFI_STEP_DONE
} AppWifiStep_t;

/**
 * @brief 清理 USART3 错误标志。
 *
 * ESP01S 上电时可能输出启动信息，串口也可能残留溢出或噪声错误。
 * 每次发送 AT 指令前先清一次错误标志，避免旧错误影响本次接收。
 */
static void AppWifi_ClearUartErrorFlags(void)
{
    __HAL_UART_CLEAR_OREFLAG(&huart3);
    __HAL_UART_CLEAR_FEFLAG(&huart3);
    __HAL_UART_CLEAR_NEFLAG(&huart3);
    __HAL_UART_CLEAR_PEFLAG(&huart3);
}


static void AppWifi_DrainRx(uint32_t drain_ms)
{
    uint8_t ch = 0U;
    uint32_t start_tick = HAL_GetTick();

    /* 发送新命令前丢弃 ESP01S 上电日志或上一条命令残留字节。 */
    while ((HAL_GetTick() - start_tick) < drain_ms) {
        (void)HAL_UART_Receive(&huart3, &ch, 1U, 5U);
    }
}

/**
 * @brief 打印 ESP01S 回复内容。
 *
 * text 用于人看，hex 用于确认 \r\n、乱码、残缺字符等不可见字节。
 */
static void AppWifi_PrintResponse(const AppWifiAtResponse_t *resp)
{
    if ((resp == NULL) || (resp->len == 0U)) {
        printf("[esp] recv none\r\n");
        return;
    }

    printf("[esp] recv len=%u text=", resp->len);
    for (uint16_t i = 0; i < resp->len; i++) {
        char out = ((resp->text[i] >= 32) && (resp->text[i] <= 126)) ? resp->text[i] : '.';
        printf("%c", out);
    }
    printf("\r\n[esp] recv hex=");
    for (uint16_t i = 0; i < resp->len; i++) {
        printf("%02X ", (uint8_t)resp->text[i]);
    }
    printf("\r\n");
}

static void AppWifi_PrintSendCommand(const char *cmd)
{
    if (cmd == NULL) {
        return;
    }

    /* CWJAP 包含 WiFi 密码，串口日志只打印脱敏版本，避免截图和 GitHub 泄露。 */
    if (strncmp(cmd, "AT+CWJAP=", 9U) == 0) {
        printf("[esp] send: AT+CWJAP=\"%s\",\"***\"\r\n", APP_WIFI_SSID);
        return;
    }

    printf("[esp] send: %s\r\n", cmd);
}

static AppWifiAtResult_t AppWifi_SendAtCommand(const char *cmd,
                                               const char *expect,
                                               uint32_t timeout_ms,
                                               AppWifiAtResponse_t *resp)
{
    char tx_line[APP_WIFI_TX_BUF_SIZE];
    uint8_t ch = 0U;

    if ((cmd == NULL) || (expect == NULL) || (resp == NULL)) {
        return APP_WIFI_AT_REPLY_ERROR;
    }

    memset(resp, 0, sizeof(*resp));
    snprintf(tx_line, sizeof(tx_line), "%s\r\n", cmd);

    AppWifi_ClearUartErrorFlags();
    AppWifi_DrainRx(APP_WIFI_AT_DRAIN_MS);

    AppWifi_PrintSendCommand(cmd);
    if (HAL_UART_Transmit(&huart3, (uint8_t *)tx_line, (uint16_t)strlen(tx_line), 100U) != HAL_OK) {
        printf("[esp] result=TX_ERR\r\n");
        return APP_WIFI_AT_TX_ERROR;
    }

    /* 接收窗口内只缓存，不 printf，避免日志输出干扰 ESP01S 回复采集。 */
    uint32_t start_tick = HAL_GetTick();
    while ((HAL_GetTick() - start_tick) < timeout_ms) {
        if (HAL_UART_Receive(&huart3, &ch, 1U, 20U) == HAL_OK) {
            if (resp->len < (APP_WIFI_RX_BUF_SIZE - 1U)) {
                resp->text[resp->len++] = (char)ch;
                resp->text[resp->len] = '\0';
            }
        }
    }

    AppWifi_PrintResponse(resp);

    if ((strstr(resp->text, "ERROR") != NULL) ||
        (strstr(resp->text, "FAIL") != NULL) ||
        (strstr(resp->text, "busy") != NULL)) {
        printf("[esp] result=REPLY_ERR\r\n");
        return APP_WIFI_AT_REPLY_ERROR;
    }

    if (strstr(resp->text, expect) != NULL) {
        printf("[esp] result=OK\r\n");
        return APP_WIFI_AT_OK;
    }

    printf("[esp] result=TIMEOUT\r\n");
    return APP_WIFI_AT_TIMEOUT;
}

/* Exported functions ------------------------------------------------------*/

void AppWifi_Init(void)
{
    AppState_t *state = AppState_Get();
    state->wifi_status = APP_WIFI_OFF;
    last_print_tick = 0;
    /* 系统启动后允许执行一次 ESP01S AT 通信测试 */
    esp_at_test_done = 0;
}

void AppWifi_Process(void) {
  uint32_t now = HAL_GetTick();

  if (esp_at_test_done != 0U) {
    return;
  }

  /* 每 3 秒执行一步 AT 指令，避免多条指令连续发送时日志和回复混在一起 */
  if ((now - last_print_tick) < 3000U) {
    return;
  }
  last_print_tick = now;

  /*
   * ESP01S 基础 AT 指令验证顺序：
   * 1. AT：确认 USART3 双向通信正常。
   * 2. ATE0：关闭回显，后续回复不再带原始 AT 指令，便于解析。
   * 3. AT+CWMODE=1：设置为 Station 模式，为后续连接 WiFi 做准备。
   */
    static AppWifiStep_t wifi_step = APP_WIFI_STEP_AT;
    static uint8_t cmd_fail_count = 0U;
    AppWifiAtResponse_t resp;
    AppWifiAtResult_t result;
    char cmd_buf[128];

    switch (wifi_step) {
    case APP_WIFI_STEP_AT:
      result = AppWifi_SendAtCommand("AT", "OK", APP_WIFI_AT_SHORT_TIMEOUT_MS,
                                     &resp);
      break;

    case APP_WIFI_STEP_ATE0:
      result = AppWifi_SendAtCommand("ATE0", "OK", APP_WIFI_AT_SHORT_TIMEOUT_MS,
                                     &resp);
      break;

    case APP_WIFI_STEP_CWMODE:
      result = AppWifi_SendAtCommand("AT+CWMODE_CUR=1", "OK",
                                     APP_WIFI_AT_MODE_TIMEOUT_MS, &resp);
      break;

    case APP_WIFI_STEP_CWJAP:
      snprintf(cmd_buf, sizeof(cmd_buf), "AT+CWJAP=\"%s\",\"%s\"",
               APP_WIFI_SSID, APP_WIFI_PASSWORD);

      result = AppWifi_SendAtCommand(cmd_buf, "WIFI GOT IP",
                                     APP_WIFI_JOIN_TIMEOUT_MS, &resp);
      break;

    case APP_WIFI_STEP_CIFSR:
      result = AppWifi_SendAtCommand("AT+CIFSR", "STAIP",
                                     APP_WIFI_IP_TIMEOUT_MS, &resp);
      break;

    case APP_WIFI_STEP_DONE:
    default:
      return;
    }

    if (result == APP_WIFI_AT_OK) {
      cmd_fail_count = 0U;

      if (wifi_step == APP_WIFI_STEP_CIFSR) {
        AppState_Get()->wifi_status = APP_WIFI_CONNECTED;
        wifi_step = APP_WIFI_STEP_DONE;
        esp_at_test_done = 1U;
        printf("[esp] wifi connected and got ip\r\n");
      } else {
        wifi_step++;
        AppState_Get()->wifi_status = APP_WIFI_CONNECTING;
      }
    } else {
      if (cmd_fail_count < APP_WIFI_AT_MAX_RETRY) {
        cmd_fail_count++;
      }

      if (cmd_fail_count >= APP_WIFI_AT_MAX_RETRY) {
        AppState_Get()->wifi_status = APP_WIFI_ERROR;
      } else {
        AppState_Get()->wifi_status = APP_WIFI_CONNECTING;
      }
    }
}
