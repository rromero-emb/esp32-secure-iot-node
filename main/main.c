#include <stdbool.h>
#include <stdio.h>

#include "dht11.h"
#include "esp_app_desc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ir_sensor.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "ssd1306.h"
#include "status_led.h"

static const char *TAG = "node";

static bool s_have_dht11;
static bool s_have_oled;

static void on_obstacle(bool obstacle, void *ctx)
{
    (void)ctx;
    ESP_LOGI(TAG, "IR sensor: %s", obstacle ? "obstacle" : "clear");
    /* Session 2: publish as an MQTT event. Session 4: wake-up source. */
}

static void init_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

/* Optional peripherals: a missing or not-yet-implemented driver is logged, not fatal. */
static bool init_optional(const char *name, esp_err_t err)
{
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "%s unavailable: %s", name, esp_err_to_name(err));
        return false;
    }
    return true;
}

static void show_reading(const dht11_reading_t *r)
{
    if (!s_have_oled) {
        return;
    }
    char line[SSD1306_TEXT_COLS + 1];
    ssd1306_clear();
    /* 16-column display: keep at most 13 characters of the version string. */
    snprintf(line, sizeof(line), "fw %.13s", esp_app_get_description()->version);
    ssd1306_draw_text(0, line);
    snprintf(line, sizeof(line), "T  %d C", r->temperature_c);
    ssd1306_draw_text(2, line);
    snprintf(line, sizeof(line), "RH %d %%", r->humidity_pct);
    ssd1306_draw_text(3, line);
    ssd1306_flush();
}

void app_main(void)
{
    const esp_app_desc_t *app = esp_app_get_description();
    ESP_LOGI(TAG, "%s %s (ESP-IDF %s)", app->project_name, app->version, app->idf_ver);

    init_nvs();
    ESP_ERROR_CHECK(status_led_init(CONFIG_NODE_STATUS_LED_GPIO));
    ESP_ERROR_CHECK(ir_sensor_init(CONFIG_NODE_IR_SENSOR_GPIO, on_obstacle, NULL));

    s_have_dht11 = init_optional("DHT11", dht11_init(CONFIG_NODE_DHT11_GPIO));
    s_have_oled = init_optional("OLED", ssd1306_init(CONFIG_NODE_I2C_SDA_GPIO,
                                                     CONFIG_NODE_I2C_SCL_GPIO,
                                                     CONFIG_NODE_OLED_I2C_ADDR));

    for (;;) {
        if (s_have_dht11) {
            dht11_reading_t r;
            const esp_err_t err = dht11_read(&r);
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "T=%d C RH=%d %%", r.temperature_c, r.humidity_pct);
                show_reading(&r);
            } else {
                ESP_LOGW(TAG, "DHT11 read failed: %s", esp_err_to_name(err));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(CONFIG_NODE_SENSOR_PERIOD_MS));
    }
}
