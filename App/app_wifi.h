/**
  ******************************************************************************
  * @file           : app_wifi.h
  * @brief          : WiFi module interface (ESP8266 placeholder)
  ******************************************************************************
  */

#ifndef APP_WIFI_H
#define APP_WIFI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported functions ------------------------------------------------------*/

/**
 * @brief Initialize WiFi module
 */
void AppWifi_Init(void);

/**
 * @brief Process WiFi state machine
 */
void AppWifi_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_WIFI_H */
