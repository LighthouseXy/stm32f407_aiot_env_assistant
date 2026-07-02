/**
  ******************************************************************************
  * @file           : app_mqtt.h
  * @brief          : MQTT client interface (placeholder)
  ******************************************************************************
  */

#ifndef APP_MQTT_H
#define APP_MQTT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported functions ------------------------------------------------------*/

/**
 * @brief Initialize MQTT client
 */
void AppMqtt_Init(void);

/**
 * @brief Process MQTT state machine
 */
void AppMqtt_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_MQTT_H */
