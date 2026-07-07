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

#include <stdint.h>

/* Exported functions ------------------------------------------------------*/

/**
 * @brief Initialize WiFi module
 */
void AppWifi_Init(void);

/**
 * @brief Process WiFi state machine
 */
void AppWifi_Process(void);

/**
 * @brief Send one ESP01S AT command and wait for expected response.
 * @param cmd AT command without trailing CRLF
 * @param expect Expected response keyword
 * @param timeout_ms Receive timeout window in milliseconds
 * @return 0 on success, -1 on failure
 */
int AppWifi_SendCommand(const char *cmd, const char *expect, uint32_t timeout_ms);

/**
 * @brief Send raw TCP payload through ESP01S after CIPSEND.
 * @param payload TCP payload buffer
 * @param len Payload length in bytes
 * @param timeout_ms Receive timeout window in milliseconds
 * @return 0 on success, -1 on failure
 */
int AppWifi_SendTcpPayload(const uint8_t *payload,
                           uint16_t len,
                           uint8_t *rx_buf,
                           uint16_t rx_max_len,
                           uint16_t *rx_len,
                           uint32_t timeout_ms);
/**
 * @brief Receive TCP data from ESP01S.
 * @param buf Receive buffer
 * @param max_len Receive buffer size
 * @param out_len Actual received length
 * @param timeout_ms Receive timeout window in milliseconds
 * @return 0 on data received, -1 on timeout or invalid parameter
 */
int AppWifi_ReceiveTcpData(uint8_t *buf,
                           uint16_t max_len,
                           uint16_t *out_len,
                           uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* APP_WIFI_H */
