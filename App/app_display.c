/**
  ******************************************************************************
  * @file           : app_display.c
  * @brief          : Display implementation (UART printf simulation)
  ******************************************************************************
  */

#include "app_display.h"
#include "app_oled.h"
#include "app_state.h"
#include "main.h"
#include <stdio.h>
#include <stddef.h>

extern I2C_HandleTypeDef hi2c1;
/* 提前声明浮点数转字符串函数，避免后面的 OLED 更新函数调用时出现隐式声明 */
static void AppDisplay_FormatFloat1(char *buf, size_t len, float value);

static const char *AppDisplay_WifiShortString(AppWifiStatus_t status)
{
    switch (status) {
        case APP_WIFI_OFF:        return "OFF";
        case APP_WIFI_CONNECTING: return "CONN";
        case APP_WIFI_CONNECTED:  return "OK";
        case APP_WIFI_ERROR:      return "ERR";
        default:                  return "UNK";
    }
}

/* OLED 实时状态刷新 */
static void AppDisplay_UpdateOled(const AppState_t *state)
{
    char line[24];
    char temp_text[12];
    char hum_text[12];

    if (state == NULL) {
      return;
    }

    AppDisplay_FormatFloat1(temp_text, sizeof(temp_text), state->temperature);
    AppDisplay_FormatFloat1(hum_text, sizeof(hum_text), state->humidity);

    AppOled_Clear();

    snprintf(line, sizeof(line), "T:%sC", temp_text);
    AppOled_ShowString(0U, 0U, line);

    snprintf(line, sizeof(line), "H:%s%%", hum_text);
    AppOled_ShowString(2U, 0U, line);

    snprintf(line, sizeof(line), "L:%d%%", state->light_percent);
    AppOled_ShowString(4U, 0U, line);

    /* OLED 第 4 行显示 WiFi 状态，方便验证 ESP01S 入网结果。 */
    snprintf(line, sizeof(line), "W:%s",
             AppDisplay_WifiShortString(state->wifi_status));
    AppOled_ShowString(6U, 0U, line);
}

static void AppDisplay_FormatFloat1(char *buf, size_t len, float value)
{
    if ((buf == NULL) || (len == 0U)) {
        return;
    }

    /* OLED 显示不依赖 printf 浮点格式化，先转成 x.y 字符串。 */
    int value_x10 = (value >= 0.0f) ?
                    (int)(value * 10.0f + 0.5f) :
                    (int)(value * 10.0f - 0.5f);

    const char *sign = (value_x10 < 0) ? "-" : "";
    unsigned int abs_x10 = (value_x10 < 0) ? (unsigned int)(-value_x10) : (unsigned int)value_x10;

    snprintf(buf,
             len,
             "%s%u.%u",
             sign,
             abs_x10 / 10U,
             abs_x10 % 10U);
}

/* Exported functions ------------------------------------------------------*/

void AppDisplay_Init(void)
{
    /* OLED 使用 I2C1，地址已通过扫描确认为 0x3C。 */
    AppOled_Init(&hi2c1);

    /* 字符显示验证：先显示固定文本，确认字模和字符串写屏正常。 */
//     AppOled_Clear();
//     AppOled_ShowString(0U, 0U, "AIoT Env");
//     AppOled_ShowString(2U, 0U, "OLED OK");
}

void AppDisplay_Update(void)
{
    AppState_t *state = AppState_Get();
    // int temp_x10 = (int)(state->temperature * 10.0f);
    // int hum_x10 = (int)(state->humidity * 10.0f);

    #if 0
    /* newlib-nano 默认不支持 printf 浮点格式，这里用整数拼出 1 位小数。 */
    printf("[display] mode=%s light_raw=%u light=%d aht=%s temp=%d.%d hum=%d.%d wifi=%s mqtt=%s\r\n",
           AppState_ModeToString(state->mode),
           (unsigned int)state->light_raw,
           state->light_percent,
           (state->aht30_valid != 0U) ? "OK" : "ERR",
           temp_x10 / 10,
           temp_x10 % 10,
           hum_x10 / 10,
           hum_x10 % 10,
           AppState_WifiToString(state->wifi_status),
           AppState_MqttToString(state->mqtt_status));
    #endif
    
    /* 串口显示和 OLED 显示共用同一份 AppState_t 状态。 */
    AppDisplay_UpdateOled(state);
}
