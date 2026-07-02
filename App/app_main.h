/**
  ******************************************************************************
  * @file           : app_main.h
  * @brief          : Application layer entry point
  ******************************************************************************
  */

#ifndef APP_MAIN_H
#define APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported functions ------------------------------------------------------*/

/**
 * @brief Initialize all application modules
 */
void App_Init(void);

/**
 * @brief Sensor task loop (call every 500ms)
 */
void App_SensorTaskLoop(void);

/**
 * @brief Display task loop (call every 1000ms)
 */
void App_DisplayTaskLoop(void);

/**
 * @brief WiFi task loop (call every 2000ms)
 */
void App_WifiTaskLoop(void);

/**
 * @brief MQTT task loop (call every 3000ms)
 */
void App_MqttTaskLoop(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_MAIN_H */
