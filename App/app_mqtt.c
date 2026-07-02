/**
  ******************************************************************************
  * @file           : app_mqtt.c
  * @brief          : MQTT client implementation (placeholder)
  ******************************************************************************
  */

#include "app_mqtt.h"
#include "app_state.h"
#include <stdio.h>

/* Private variables -------------------------------------------------------*/
static uint32_t last_print_tick = 0;

/* Exported functions ------------------------------------------------------*/

void AppMqtt_Init(void)
{
    AppState_t *state = AppState_Get();
    state->mqtt_status = APP_MQTT_OFF;
    last_print_tick = 0;
}

void AppMqtt_Process(void)
{
    AppState_t *state = AppState_Get();

    /* Placeholder: print status every 5 seconds */
    uint32_t now = state->tick_ms;
    if ((now - last_print_tick) >= 5000) {
        printf("[mqtt] pending MQTT connection\r\n");
        last_print_tick = now;
    }
}
