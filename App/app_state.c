/**
  ******************************************************************************
  * @file           : app_state.c
  * @brief          : Application state management implementation
  ******************************************************************************
  */

#include "app_state.h"
#include <string.h>

/* Private variables -------------------------------------------------------*/
static AppState_t g_app_state;

/* Exported functions ------------------------------------------------------*/

void AppState_Init(void)
{
    memset(&g_app_state, 0, sizeof(AppState_t));
    g_app_state.mode = APP_MODE_NORMAL;
    g_app_state.wifi_status = APP_WIFI_OFF;
    g_app_state.mqtt_status = APP_MQTT_OFF;
    g_app_state.alert_active = 0;
}

AppState_t *AppState_Get(void)
{
    return &g_app_state;
}

void AppState_SetMode(AppMode_t mode)
{
    g_app_state.mode = mode;
}

const char *AppState_ModeToString(AppMode_t mode)
{
    switch (mode) {
        case APP_MODE_NORMAL: return "NORMAL";
        case APP_MODE_FOCUS:  return "FOCUS";
        case APP_MODE_ALERT:  return "ALERT";
        default:              return "UNKNOWN";
    }
}

const char *AppState_WifiToString(AppWifiStatus_t status)
{
    switch (status) {
        case APP_WIFI_OFF:        return "OFF";
        case APP_WIFI_CONNECTING: return "CONNECTING";
        case APP_WIFI_CONNECTED:  return "CONNECTED";
        case APP_WIFI_ERROR:      return "ERROR";
        default:                  return "UNKNOWN";
    }
}

const char *AppState_MqttToString(AppMqttStatus_t status)
{
    switch (status) {
        case APP_MQTT_OFF:        return "OFF";
        case APP_MQTT_CONNECTING: return "CONNECTING";
        case APP_MQTT_CONNECTED:  return "CONNECTED";
        case APP_MQTT_ERROR:      return "ERROR";
        default:                  return "UNKNOWN";
    }
}
