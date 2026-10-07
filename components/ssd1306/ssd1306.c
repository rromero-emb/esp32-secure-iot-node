#include "ssd1306.h"

/*
 * TODO (session 1): driver on the new I2C master API (driver/i2c_master.h).
 *  - init sequence (charge pump, addressing mode, contrast), 1 KB frame buffer,
 *    8x8 font, flush in a single I2C transaction per page.
 */

esp_err_t ssd1306_init(int sda_gpio, int scl_gpio, uint8_t i2c_addr)
{
    (void)sda_gpio;
    (void)scl_gpio;
    (void)i2c_addr;
    return ESP_ERR_NOT_SUPPORTED;
}

void ssd1306_clear(void)
{
}

void ssd1306_draw_text(int row, const char *text)
{
    (void)row;
    (void)text;
}

esp_err_t ssd1306_flush(void)
{
    return ESP_ERR_NOT_SUPPORTED;
}
