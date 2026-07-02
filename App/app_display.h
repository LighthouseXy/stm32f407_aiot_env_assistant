/**
  ******************************************************************************
  * @file           : app_display.h
  * @brief          : Display interface (OLED placeholder)
  ******************************************************************************
  */

#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported functions ------------------------------------------------------*/

/**
 * @brief Initialize display module
 */
void AppDisplay_Init(void);

/**
 * @brief Update display (currently UART printf simulation)
 */
void AppDisplay_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_DISPLAY_H */
