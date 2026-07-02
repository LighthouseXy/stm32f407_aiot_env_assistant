/**
  ******************************************************************************
  * @file           : app_state.h
  * @brief          : Application state management
  ******************************************************************************
  */

#ifndef APP_STATE_H
#define APP_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Exported types ----------------------------------------------------------*/

/**
 * @brief Application work modes
 */
typedef enum {
    APP_MODE_NORMAL = 0,   /**< Normal operation mode */
    APP_MODE_FOCUS,        /**< Focus/study mode */
    APP_MODE_ALERT         /**< Alert mode */
} AppMode_t;

/**
 * @brief WiFi connection status
 */
typedef enum {
    APP_WIFI_OFF = 0,      /**< WiFi disabled */
    APP_WIFI_CONNECTING,   /**< WiFi connecting */
    APP_WIFI_CONNECTED,    /**< WiFi connected */
    APP_WIFI_ERROR         /**< WiFi error */
} AppWifiStatus_t;

/**
 * @brief MQTT connection status
 */
typedef enum {
    APP_MQTT_OFF = 0,      /**< MQTT disabled */
    APP_MQTT_CONNECTING,   /**< MQTT connecting */
    APP_MQTT_CONNECTED,    /**< MQTT connected */
    APP_MQTT_ERROR         /**< MQTT error */
} AppMqttStatus_t;

/**
 * @brief System state structure
 */
typedef struct {
    uint32_t tick_ms;              /**< System tick in milliseconds */
    AppMode_t mode;                /**< Current work mode */
    int light_percent;             /**< Light sensor percentage (0-100) */
    uint16_t light_raw;             /**< Raw ADC value from light sensor (0-4095) */
    float temperature;             /**< Temperature in Celsius */
    float humidity;                /**< Humidity percentage */
    uint8_t aht30_valid;           /**< AHT30 current sample flag (1=valid, 0=invalid) */
    AppWifiStatus_t wifi_status;   /**< WiFi connection status */
    AppMqttStatus_t mqtt_status;   /**< MQTT connection status */
    uint8_t alert_active;          /**< Alert flag (1=active, 0=inactive) */
} AppState_t;

/* Exported functions ------------------------------------------------------*/

/**
 * @brief Initialize application state
 */
void AppState_Init(void);

/**
 * @brief Get pointer to application state
 * @return Pointer to global AppState_t
 */
AppState_t *AppState_Get(void);

/**
 * @brief Set application work mode
 * @param mode New work mode
 */
void AppState_SetMode(AppMode_t mode);

/**
 * @brief Convert work mode to string
 * @param mode Work mode
 * @return String representation
 */
const char *AppState_ModeToString(AppMode_t mode);

/**
 * @brief Convert WiFi status to string
 * @param status WiFi status
 * @return String representation
 */
const char *AppState_WifiToString(AppWifiStatus_t status);

/**
 * @brief Convert MQTT status to string
 * @param status MQTT status
 * @return String representation
 */
const char *AppState_MqttToString(AppMqttStatus_t status);

#ifdef __cplusplus
}
#endif

#endif /* APP_STATE_H */
