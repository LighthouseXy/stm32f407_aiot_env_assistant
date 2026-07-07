/**
  ******************************************************************************
  * @file           : app_sensor.c
  * @brief          : 传感器数据采集
  ******************************************************************************
  */

#include "app_sensor.h"
#include "app_aht30.h"
#include "app_state.h"
#include "main.h"
#include <stdio.h>

extern ADC_HandleTypeDef hadc3;
extern I2C_HandleTypeDef hi2c1;
/* Private variables -------------------------------------------------------*/
static uint8_t aht30_ready = 0U;
static HAL_StatusTypeDef aht30_probe_status = HAL_ERROR;
static HAL_StatusTypeDef aht30_init_status = HAL_ERROR;
static HAL_StatusTypeDef aht30_read_status = HAL_ERROR;
static uint32_t last_sensor_debug_tick = 0U;

/* Exported functions ------------------------------------------------------*/

static uint16_t AppSensor_ReadLightRaw(void)
{
    uint16_t raw = 0;
    /*
     * 板载光敏传感器 LS1 读取真实光照电压。
     */
    if (HAL_ADC_Start(&hadc3) == HAL_OK) {
        if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK) {
            raw = (uint16_t)HAL_ADC_GetValue(&hadc3);
        }
        /* 单次采样结束后停止 ADC，下一轮任务再重新启动采样。 */
        HAL_ADC_Stop(&hadc3);
    }
    return raw;
}

static int AppSensor_RawToPercent(uint16_t raw)
{
    uint32_t raw_percent = ((uint32_t)raw * 100U) / APP_SENSOR_ADC_MAX_VALUE;

    /*
     * 板载光敏电路的 ADC 电压与“亮度直觉”相反：
     * 当前实测为环境越暗 raw 越大，环境越亮 raw 越小。
     * OLED 显示的 L:% 希望表达“亮度百分比”，因此这里做一次反向换算。
     */
    if (raw_percent > 100U) {
        raw_percent = 100U;
    }

    return (int)(100U - raw_percent);
}

void AppSensor_Init(void)
{
    /* AHT30 接在 CubeMX 配置的 I2C1 上：PB6=SCL，PB7=SDA。 */
    aht30_probe_status = AHT30_IsReady(&hi2c1);
    aht30_init_status = (aht30_probe_status == HAL_OK) ? AHT30_Init(&hi2c1) : HAL_ERROR;
    aht30_ready = ((aht30_probe_status == HAL_OK) && (aht30_init_status == HAL_OK)) ? 1U : 0U;
}

void AppSensor_Update(void)
{
    AppState_t *state = AppState_Get();
    AHT30_Data_t aht30_data;

    uint16_t light_raw = AppSensor_ReadLightRaw();
    int light_percent = AppSensor_RawToPercent(light_raw);

    /* 每轮先探测地址 ACK：没有 ACK 时优先检查 VCC/GND/SCL/SDA/实际引脚。 */
    aht30_probe_status = AHT30_IsReady(&hi2c1);
    if (aht30_probe_status != HAL_OK) {
        aht30_ready = 0U;
    }

    /* 如果上电时 AHT30 还没准备好，任务运行中在探测成功后继续尝试初始化。 */
    if ((aht30_ready == 0U) && (aht30_probe_status == HAL_OK)) {
        aht30_init_status = AHT30_Init(&hi2c1);
        aht30_ready = (aht30_init_status == HAL_OK) ? 1U : 0U;
    }

    /* AHT30 读取成功后更新真实温湿度；失败则保留上一次状态并置告警标志。 */
    aht30_read_status = HAL_ERROR;
    if ((aht30_ready != 0U) && (aht30_probe_status == HAL_OK)) {
        aht30_read_status = AHT30_Read(&hi2c1, &aht30_data);
    }
    if (aht30_read_status == HAL_OK) {
        state->temperature = aht30_data.temperature_c;
        state->humidity = aht30_data.humidity_percent;
        state->aht30_valid = 1U;
    } else {
        state->aht30_valid = 0U;
        state->alert_active = 1U;
    }

    /* 更新板载光敏传感器数据：raw 为 ADC 原始值，percent 用于串口和 OLED 直观显示。 */
    state->light_raw = light_raw;
    state->light_percent = light_percent;

    /* 诊断日志限频输出，避免串口刷屏；HAL 状态：0=OK，1=ERROR，2=BUSY，3=TIMEOUT。 */
    if ((state->tick_ms - last_sensor_debug_tick) >= 10000U) {
        int temp_x10 = (int)(state->temperature * 10.0f);
        int hum_x10 = (int)(state->humidity * 10.0f);

        printf("[sensor] aht=%s probe=%d init=%d read=%d ready=%u temp=%d.%d hum=%d.%d i2c_err=0x%08lX\r\n",
               (state->aht30_valid != 0U) ? "OK" : "ERR",
               (int)aht30_probe_status,
               (int)aht30_init_status,
               (int)aht30_read_status,
               (unsigned int)aht30_ready,
               temp_x10 / 10,
               temp_x10 % 10,
               hum_x10 / 10,
               hum_x10 % 10,
               hi2c1.ErrorCode);
        last_sensor_debug_tick = state->tick_ms;
    }
}
