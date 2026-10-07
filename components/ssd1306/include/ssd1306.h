#pragma once

#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 128x64 monochrome OLED over I2C: 8 text rows of 16 characters (8x8 font). */
#define SSD1306_TEXT_ROWS 8
#define SSD1306_TEXT_COLS 16

esp_err_t ssd1306_init(int sda_gpio, int scl_gpio, uint8_t i2c_addr);

/** Clear the frame buffer (does not touch the panel until ssd1306_flush()). */
void ssd1306_clear(void);

/** Draw ASCII @p text on text row @p row (0..7), truncated to 16 characters. */
void ssd1306_draw_text(int row, const char *text);

/** Send the frame buffer to the panel. */
esp_err_t ssd1306_flush(void);

#ifdef __cplusplus
}
#endif
