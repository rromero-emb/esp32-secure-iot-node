#include "ssd1306.h"

#include <stdbool.h>
#include <string.h>

#include "driver/i2c_master.h"
#include "esp_check.h"
#include "font8x8.h"

#define WIDTH          128
#define PAGES          8      /* 64 rows / 8 rows per GDDRAM page */
#define I2C_SPEED_HZ   400000
#define I2C_TIMEOUT_MS 100

#define CTRL_CMD  0x00 /* Co = 0, D/C# = 0: a stream of command bytes follows */
#define CTRL_DATA 0x40 /* Co = 0, D/C# = 1: a stream of GDDRAM bytes follows */

static const char *TAG = "ssd1306";

static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_dev;

/* One page per row; byte 0 holds the control byte so a page is a single I2C write. */
static uint8_t s_fb[PAGES][1 + WIDTH];

static const uint8_t k_init_cmds[] = {
    CTRL_CMD,
    0xAE,       /* display off */
    0xD5, 0x80, /* clock divide ratio / oscillator frequency: reset value */
    0xA8, 0x3F, /* multiplex ratio: 64 rows */
    0xD3, 0x00, /* display offset: 0 */
    0x40,       /* display start line: 0 */
    0x8D, 0x14, /* charge pump on: these modules have no external panel supply */
    0x20, 0x00, /* horizontal addressing: the RAM pointer wraps to the next page */
    0xA1,       /* segment remap and ...                                   */
    0xC8,       /* ... reversed COM scan: usual orientation, header pins on top */
    0xDA, 0x12, /* COM pins: alternative configuration (128x64 panels) */
    0x81, 0xCF, /* contrast */
    0xD9, 0xF1, /* pre-charge period for the internal charge pump */
    0xDB, 0x40, /* VCOMH deselect level */
    0x2E,       /* scrolling off */
    0xA4,       /* display follows RAM content */
    0xA6,       /* normal, not inverted */
    0xAF,       /* display on */
};

/* Whole-screen window: a flush always rewrites the 8 pages in order. */
static const uint8_t k_window_cmds[] = {
    CTRL_CMD,
    0x21, 0, WIDTH - 1, /* column range */
    0x22, 0, PAGES - 1, /* page range */
};

esp_err_t ssd1306_init(int sda_gpio, int scl_gpio, uint8_t i2c_addr)
{
    ESP_RETURN_ON_FALSE(!s_bus, ESP_ERR_INVALID_STATE, TAG, "already initialised");

    esp_err_t ret = ESP_OK;
    const i2c_master_bus_config_t bus_cfg = {
        .i2c_port = -1, /* any free controller */
        .sda_io_num = sda_gpio,
        .scl_io_num = scl_gpio,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true, /* most modules also carry their own */
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &s_bus), TAG, "i2c_new_master_bus failed");

    /* No panel: report it and let the node run without a display. */
    ESP_GOTO_ON_ERROR(i2c_master_probe(s_bus, i2c_addr, I2C_TIMEOUT_MS), fail, TAG,
                      "no device at 0x%02x", i2c_addr);

    const i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = i2c_addr,
        .scl_speed_hz = I2C_SPEED_HZ,
    };
    ESP_GOTO_ON_ERROR(i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev), fail, TAG,
                      "add device failed");
    ESP_GOTO_ON_ERROR(i2c_master_transmit(s_dev, k_init_cmds, sizeof(k_init_cmds), I2C_TIMEOUT_MS),
                      fail, TAG, "init sequence failed");

    ssd1306_clear();
    ESP_GOTO_ON_ERROR(ssd1306_flush(), fail, TAG, "first flush failed");
    return ESP_OK;

fail:
    if (s_dev) {
        i2c_master_bus_rm_device(s_dev);
        s_dev = NULL;
    }
    i2c_del_master_bus(s_bus);
    s_bus = NULL;
    return ret;
}

void ssd1306_clear(void)
{
    memset(s_fb, 0, sizeof(s_fb));
}

void ssd1306_draw_text(int row, const char *text)
{
    if (row < 0 || row >= SSD1306_TEXT_ROWS || !text) {
        return;
    }
    uint8_t *page = &s_fb[row][1];

    for (int col = 0; col < SSD1306_TEXT_COLS && text[col]; col++) {
        uint8_t c = (uint8_t)text[col];
        if (c < FONT8X8_FIRST || c > FONT8X8_LAST) {
            c = '?';
        }
        const uint8_t *glyph = font8x8_basic[c - FONT8X8_FIRST];

        /*
         * The font is stored row by row (bit 0 = leftmost pixel), while a GDDRAM
         * byte is one column of a page (bit 0 = top pixel): transpose the 8x8 cell.
         */
        for (int x = 0; x < 8; x++) {
            uint8_t column = 0;
            for (int y = 0; y < 8; y++) {
                column |= (uint8_t)(((glyph[y] >> x) & 1) << y);
            }
            page[col * 8 + x] = column;
        }
    }
}

esp_err_t ssd1306_flush(void)
{
    ESP_RETURN_ON_FALSE(s_dev, ESP_ERR_INVALID_STATE, TAG, "not initialised");

    ESP_RETURN_ON_ERROR(i2c_master_transmit(s_dev, k_window_cmds, sizeof(k_window_cmds),
                                            I2C_TIMEOUT_MS),
                        TAG, "set window failed");
    for (int p = 0; p < PAGES; p++) {
        s_fb[p][0] = CTRL_DATA;
        ESP_RETURN_ON_ERROR(i2c_master_transmit(s_dev, s_fb[p], sizeof(s_fb[p]), I2C_TIMEOUT_MS),
                            TAG, "page %d write failed", p);
    }
    return ESP_OK;
}
