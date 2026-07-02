#include "app_oled.h"
#include <string.h>

#define APP_OLED_I2C_ADDR_HAL   (APP_OLED_I2C_ADDR_7BIT << 1)
#define APP_OLED_CMD_CONTROL    0x00U
#define APP_OLED_DATA_CONTROL   0x40U
#define APP_OLED_I2C_TIMEOUT_MS 100U

static I2C_HandleTypeDef *s_oled_i2c = NULL;
static uint8_t s_oled_ready = 0U;

static HAL_StatusTypeDef AppOled_WriteCommand(uint8_t cmd)
{
  uint8_t tx[2] = {APP_OLED_CMD_CONTROL, cmd};

  if (s_oled_i2c == NULL) {
    return HAL_ERROR;
  }

  return HAL_I2C_Master_Transmit(s_oled_i2c,
                                 APP_OLED_I2C_ADDR_HAL,
                                 tx,
                                 sizeof(tx),
                                 APP_OLED_I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef AppOled_WriteData(const uint8_t *data, uint16_t len)
{
  uint8_t tx[17];

  if ((s_oled_i2c == NULL) || (data == NULL)) {
    return HAL_ERROR;
  }

  while (len > 0U) {
    uint16_t chunk = (len > 16U) ? 16U : len;

    tx[0] = APP_OLED_DATA_CONTROL;
    memcpy(&tx[1], data, chunk);

    if (HAL_I2C_Master_Transmit(s_oled_i2c,
                                APP_OLED_I2C_ADDR_HAL,
                                tx,
                                (uint16_t)(chunk + 1U),
                                APP_OLED_I2C_TIMEOUT_MS) != HAL_OK) {
      return HAL_ERROR;
    }

    data += chunk;
    len -= chunk;
  }

  return HAL_OK;
}

static void AppOled_SetCursor(uint8_t page, uint8_t column)
{
  /* SSD1306 按页写显存：page 是 0~7，column 是 0~127。 */
  AppOled_WriteCommand((uint8_t)(0xB0U + page));
  AppOled_WriteCommand((uint8_t)(0x00U + (column & 0x0FU)));
  AppOled_WriteCommand((uint8_t)(0x10U + ((column >> 4) & 0x0FU)));
}

/* 最小字符字模查询函数
 * 说明：
 * 1. 每个字符占 6 列，前 5 列是 5x7 字模，第 6 列留空用于字符间距。
 * 2. OLED 当前只需要显示英文、数字和少量符号，所以先做轻量级字模表。
 * 3. 如果传入未支持字符，默认显示空格，避免 OLED 出现乱码。
 */
static const uint8_t *AppOled_GetGlyph(char ch)
{
  static const uint8_t glyph_space[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  /* 数字字模：用于显示温度、湿度、光照百分比等数值。 */
  static const uint8_t glyph_0[6] = {0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00};
  static const uint8_t glyph_1[6] = {0x00, 0x42, 0x7F, 0x40, 0x00, 0x00};
  static const uint8_t glyph_2[6] = {0x42, 0x61, 0x51, 0x49, 0x46, 0x00};
  static const uint8_t glyph_3[6] = {0x21, 0x41, 0x45, 0x4B, 0x31, 0x00};
  static const uint8_t glyph_4[6] = {0x18, 0x14, 0x12, 0x7F, 0x10, 0x00};
  static const uint8_t glyph_5[6] = {0x27, 0x45, 0x45, 0x45, 0x39, 0x00};
  static const uint8_t glyph_6[6] = {0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00};
  static const uint8_t glyph_7[6] = {0x01, 0x71, 0x09, 0x05, 0x03, 0x00};
  static const uint8_t glyph_8[6] = {0x36, 0x49, 0x49, 0x49, 0x36, 0x00};
  static const uint8_t glyph_9[6] = {0x06, 0x49, 0x49, 0x29, 0x1E, 0x00};

  /* 常用符号字模：用于 T:34.5C、H:78.0%、L:64% 这类显示格式。 */
  static const uint8_t glyph_colon[6]   = {0x00, 0x36, 0x36, 0x00, 0x00, 0x00};
  static const uint8_t glyph_dot[6]     = {0x00, 0x60, 0x60, 0x00, 0x00, 0x00};
  static const uint8_t glyph_percent[6] = {0x23, 0x13, 0x08, 0x64, 0x62, 0x00};
  static const uint8_t glyph_minus[6]   = {0x08, 0x08, 0x08, 0x08, 0x08, 0x00};

  /* 大写英文字模：用于 NORMAL、FOCUS、ALERT、AHT、OLED 等状态文本。 */
  static const uint8_t glyph_A[6] = {0x7E, 0x09, 0x09, 0x09, 0x7E, 0x00};
  static const uint8_t glyph_C[6] = {0x3E, 0x41, 0x41, 0x41, 0x22, 0x00};
  static const uint8_t glyph_D[6] = {0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00};
  static const uint8_t glyph_E[6] = {0x7F, 0x49, 0x49, 0x49, 0x41, 0x00};
  static const uint8_t glyph_F[6] = {0x7F, 0x09, 0x09, 0x09, 0x01, 0x00};
  static const uint8_t glyph_H[6] = {0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00};
  static const uint8_t glyph_I[6] = {0x00, 0x41, 0x7F, 0x41, 0x00, 0x00};
  static const uint8_t glyph_K[6] = {0x7F, 0x08, 0x14, 0x22, 0x41, 0x00};
  static const uint8_t glyph_L[6] = {0x7F, 0x40, 0x40, 0x40, 0x40, 0x00};
  static const uint8_t glyph_M[6] = {0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00};
  static const uint8_t glyph_N[6] = {0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00};
  static const uint8_t glyph_O[6] = {0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00};
  static const uint8_t glyph_R[6] = {0x7F, 0x09, 0x19, 0x29, 0x46, 0x00};
  static const uint8_t glyph_S[6] = {0x46, 0x49, 0x49, 0x49, 0x31, 0x00};
  static const uint8_t glyph_T[6] = {0x01, 0x01, 0x7F, 0x01, 0x01, 0x00};
  static const uint8_t glyph_U[6] = {0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00};

  /* 小写英文字模：保留原来 AIoT Env 测试字符串需要的字符。 */
  static const uint8_t glyph_e[6] = {0x38, 0x54, 0x54, 0x54, 0x18, 0x00};
  static const uint8_t glyph_n[6] = {0x7C, 0x08, 0x04, 0x04, 0x78, 0x00};
  static const uint8_t glyph_o[6] = {0x38, 0x44, 0x44, 0x44, 0x38, 0x00};
  static const uint8_t glyph_v[6] = {0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00};

  switch (ch) {
    case ' ': return glyph_space;

    case '0': return glyph_0;
    case '1': return glyph_1;
    case '2': return glyph_2;
    case '3': return glyph_3;
    case '4': return glyph_4;
    case '5': return glyph_5;
    case '6': return glyph_6;
    case '7': return glyph_7;
    case '8': return glyph_8;
    case '9': return glyph_9;

    case ':': return glyph_colon;
    case '.': return glyph_dot;
    case '%': return glyph_percent;
    case '-': return glyph_minus;

    case 'A': return glyph_A;
    case 'C': return glyph_C;
    case 'D': return glyph_D;
    case 'E': return glyph_E;
    case 'F': return glyph_F;
    case 'H': return glyph_H;
    case 'I': return glyph_I;
    case 'K': return glyph_K;
    case 'L': return glyph_L;
    case 'M': return glyph_M;
    case 'N': return glyph_N;
    case 'O': return glyph_O;
    case 'R': return glyph_R;
    case 'S': return glyph_S;
    case 'T': return glyph_T;
    case 'U': return glyph_U;

    case 'e': return glyph_e;
    case 'n': return glyph_n;
    case 'o': return glyph_o;
    case 'v': return glyph_v;

    default:  return glyph_space;
  }
}
void AppOled_ShowChar(uint8_t page, uint8_t column, char ch)
{
  const uint8_t *glyph = AppOled_GetGlyph(ch);
  uint16_t writable_len;

  if ((s_oled_ready == 0U) || (page >= APP_OLED_PAGES) || (column >= APP_OLED_WIDTH)) {
    return;
  }

  writable_len = (uint16_t)(APP_OLED_WIDTH - column);
  if (writable_len > 6U) {
    writable_len = 6U;
  }

  AppOled_SetCursor(page, column);
  AppOled_WriteData(glyph, writable_len);
}

void AppOled_ShowString(uint8_t page, uint8_t column, const char *text)
{
  if ((s_oled_ready == 0U) || (text == NULL)) {
    return;
  }

  while ((*text != '\0') && (column < APP_OLED_WIDTH)) {
    AppOled_ShowChar(page, column, *text);
    column = (uint8_t)(column + 6U);
    text++;
  }
}

void AppOled_Init(I2C_HandleTypeDef *hi2c)
{
  s_oled_i2c = hi2c;
  s_oled_ready = 0U;

  if (s_oled_i2c == NULL) {
    return;
  }

  HAL_Delay(100);

  /* SSD1306 常用初始化序列：关闭显示 -> 配置寻址/扫描方向/对比度 -> 打开显示。 */
  AppOled_WriteCommand(0xAE);
  AppOled_WriteCommand(0x20);
  AppOled_WriteCommand(0x00);
  AppOled_WriteCommand(0xB0);
  AppOled_WriteCommand(0xC8);
  AppOled_WriteCommand(0x00);
  AppOled_WriteCommand(0x10);
  AppOled_WriteCommand(0x40);
  AppOled_WriteCommand(0x81);
  AppOled_WriteCommand(0x7F);
  AppOled_WriteCommand(0xA1);
  AppOled_WriteCommand(0xA6);
  AppOled_WriteCommand(0xA8);
  AppOled_WriteCommand(0x3F);
  AppOled_WriteCommand(0xA4);
  AppOled_WriteCommand(0xD3);
  AppOled_WriteCommand(0x00);
  AppOled_WriteCommand(0xD5);
  AppOled_WriteCommand(0x80);
  AppOled_WriteCommand(0xD9);
  AppOled_WriteCommand(0xF1);
  AppOled_WriteCommand(0xDA);
  AppOled_WriteCommand(0x12);
  AppOled_WriteCommand(0xDB);
  AppOled_WriteCommand(0x40);
  AppOled_WriteCommand(0x8D);
  AppOled_WriteCommand(0x14);
  AppOled_WriteCommand(0xAF);

  s_oled_ready = 1U;
  AppOled_Clear();
}

void AppOled_Clear(void)
{
  uint8_t line[APP_OLED_WIDTH];

  if (s_oled_ready == 0U) {
    return;
  }

  memset(line, 0x00, sizeof(line));

  for (uint8_t page = 0U; page < APP_OLED_PAGES; page++) {
    AppOled_SetCursor(page, 0U);
    AppOled_WriteData(line, sizeof(line));
  }
}

void AppOled_FillTestPattern(void)
{
  uint8_t line[APP_OLED_WIDTH];

  if (s_oled_ready == 0U) {
    return;
  }

  for (uint8_t page = 0U; page < APP_OLED_PAGES; page++) {
    memset(line, (page % 2U) == 0U ? 0xAAU : 0x55U, sizeof(line));
    AppOled_SetCursor(page, 0U);
    AppOled_WriteData(line, sizeof(line));
  }
}
