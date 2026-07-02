/**
  ******************************************************************************
  * @file           : app_main.c
  * @brief          : Application layer entry point implementation
  ******************************************************************************
  */

#include "app_main.h"
#include "app_state.h"
#include "app_sensor.h"
#include "app_display.h"
#include "app_wifi.h"
#include "app_mqtt.h"

/* Exported functions ------------------------------------------------------*/

void App_Init(void)
{
    AppState_Init();
    AppSensor_Init();
    AppDisplay_Init();
    AppWifi_Init();
    AppMqtt_Init();
}

void App_SensorTaskLoop(void)
{
    AppState_t *state = AppState_Get();
    state->tick_ms += 500;  /* Update tick based on task period */
    AppSensor_Update();
}

void App_DisplayTaskLoop(void)
{
    AppDisplay_Update();
}

void App_WifiTaskLoop(void)
{
    AppWifi_Process();
}

void App_MqttTaskLoop(void)
{
    AppMqtt_Process();
}
