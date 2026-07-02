/**
  ******************************************************************************
  * @file           : app_sensor.h
  * @brief          : Sensor data acquisition interface
  ******************************************************************************
  */

#ifndef APP_SENSOR_H
#define APP_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported constants ------------------------------------------------------*/

/* 12-bit ADC maximum raw value (2^12 - 1) */
#define APP_SENSOR_ADC_MAX_VALUE  4095U

/* Exported functions ------------------------------------------------------*/

/**
 * @brief Initialize sensor module
 */
void AppSensor_Init(void);

/**
 * @brief Update sensor data
 * @note  Reads real AHT30 temperature/humidity data and updates AppState
 */
void AppSensor_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_SENSOR_H */
