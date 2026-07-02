#ifndef APP_OLED_H
#define APP_OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* OLED 基础参数：当前 I2C 扫描确认 OLED 地址为 0x3C。 */
#define APP_OLED_I2C_ADDR_7BIT  0x3CU
#define APP_OLED_WIDTH          128U
#define APP_OLED_HEIGHT         64U
#define APP_OLED_PAGES          8U

void AppOled_Init(I2C_HandleTypeDef *hi2c);
void AppOled_Clear(void);
void AppOled_FillTestPattern(void);
void AppOled_ShowChar(uint8_t page, uint8_t column, char ch);
void AppOled_ShowString(uint8_t page, uint8_t column, const char *text);

#ifdef __cplusplus
}
#endif

#endif /* APP_OLED_H */
