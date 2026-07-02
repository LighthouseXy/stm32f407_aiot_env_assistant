/**
  ******************************************************************************
  * @file           : app_aht30.c
  * @brief          : AHT30 温湿度传感器驱动实现
  ******************************************************************************
  */

#include "app_aht30.h"

/* Private defines ---------------------------------------------------------*/

#define AHT30_I2C_ADDRESS          (0x38U << 1)  /* HAL 使用左移 1 位后的 8-bit 地址 */
#define AHT30_CMD_INIT             0xBEU         /* 初始化/校准命令 */
#define AHT30_CMD_TRIGGER          0xACU         /* 触发一次温湿度测量 */
#define AHT30_STATUS_BUSY          0x80U         /* 状态字 bit7 为 1 表示传感器忙 */
#define AHT30_RAW_FULL_SCALE       1048576.0f    /* 20-bit 原始值满量程：2^20 */
#define AHT30_I2C_TIMEOUT_MS       100U
#define AHT30_MEASURE_DELAY_MS     80U           /* 等待 AHT30 完成一次测量 */

/* Exported functions ------------------------------------------------------*/

HAL_StatusTypeDef AHT30_IsReady(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL) {
        return HAL_ERROR;
    }

    /* 先探测 0x38 地址是否有 ACK，用来区分接线/引脚问题和数据解析问题。 */
    return HAL_I2C_IsDeviceReady(hi2c,
                                 AHT30_I2C_ADDRESS,
                                 3U,
                                 AHT30_I2C_TIMEOUT_MS);
}

HAL_StatusTypeDef AHT30_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t cmd[3] = {AHT30_CMD_INIT, 0x08U, 0x00U};

    if (hi2c == NULL) {
        return HAL_ERROR;
    }

    /* 上电后先等待传感器内部完成初始化，再发送初始化命令。 */
    HAL_Delay(40U);

    return HAL_I2C_Master_Transmit(hi2c,
                                   AHT30_I2C_ADDRESS,
                                   cmd,
                                   sizeof(cmd),
                                   AHT30_I2C_TIMEOUT_MS);
}

HAL_StatusTypeDef AHT30_Read(I2C_HandleTypeDef *hi2c, AHT30_Data_t *data)
{
    uint8_t cmd[3] = {AHT30_CMD_TRIGGER, 0x33U, 0x00U};
    uint8_t rx[6] = {0};
    uint32_t raw_humidity = 0;
    uint32_t raw_temperature = 0;
    HAL_StatusTypeDef status;

    if ((hi2c == NULL) || (data == NULL)) {
        return HAL_ERROR;
    }

    status = HAL_I2C_Master_Transmit(hi2c,
                                     AHT30_I2C_ADDRESS,
                                     cmd,
                                     sizeof(cmd),
                                     AHT30_I2C_TIMEOUT_MS);
    if (status != HAL_OK) {
        return status;
    }

    /* 触发测量后等待转换完成，再读取 6 字节结果。 */
    HAL_Delay(AHT30_MEASURE_DELAY_MS);

    status = HAL_I2C_Master_Receive(hi2c,
                                    AHT30_I2C_ADDRESS,
                                    rx,
                                    sizeof(rx),
                                    AHT30_I2C_TIMEOUT_MS);
    if (status != HAL_OK) {
        return status;
    }

    /* 如果忙标志仍然为 1，本次数据不可用。 */
    if ((rx[0] & AHT30_STATUS_BUSY) != 0U) {
        return HAL_BUSY;
    }

    /* 湿度和温度各占 20 bit，跨字节拼接。 */
    raw_humidity = ((uint32_t)rx[1] << 12) |
                   ((uint32_t)rx[2] << 4) |
                   ((uint32_t)rx[3] >> 4);

    raw_temperature = (((uint32_t)rx[3] & 0x0FU) << 16) |
                      ((uint32_t)rx[4] << 8) |
                      (uint32_t)rx[5];

    /* AHT30 数据手册公式：RH = raw / 2^20 * 100，T = raw / 2^20 * 200 - 50。 */
    data->humidity_percent = ((float)raw_humidity * 100.0f) / AHT30_RAW_FULL_SCALE;
    data->temperature_c = (((float)raw_temperature * 200.0f) / AHT30_RAW_FULL_SCALE) - 50.0f;

    return HAL_OK;
}
