/**
  ******************************************************************************
  * @file           : app_aht30.h
  * @brief          : AHT30 温湿度传感器驱动接口
  ******************************************************************************
  */

#ifndef APP_AHT30_H
#define APP_AHT30_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* Exported types ----------------------------------------------------------*/

typedef struct {
    float temperature_c;       /**< 摄氏温度 */
    float humidity_percent;    /**< 相对湿度百分比 */
} AHT30_Data_t;

/* Exported functions ------------------------------------------------------*/

/**
 * @brief 检查 AHT30 是否在 I2C 总线上应答。
 * @param hi2c CubeMX 生成的 I2C 句柄
 * @return HAL_OK 表示设备地址有 ACK 响应
 */
HAL_StatusTypeDef AHT30_IsReady(I2C_HandleTypeDef *hi2c);

/**
 * @brief 通过 I2C 初始化 AHT30。
 * @param hi2c CubeMX 生成的 I2C 句柄
 * @return I2C 通信状态
 */
HAL_StatusTypeDef AHT30_Init(I2C_HandleTypeDef *hi2c);

/**
 * @brief 触发一次 AHT30 测量，并读取转换后的温湿度数据。
 * @param hi2c CubeMX 生成的 I2C 句柄
 * @param data 温湿度输出结构体
 * @return HAL_OK 表示本次测量读取成功
 */
HAL_StatusTypeDef AHT30_Read(I2C_HandleTypeDef *hi2c, AHT30_Data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* APP_AHT30_H */
