#include "status_led.h"

#include <stdbool.h>

#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_timer.h"

static const char *TAG = "status_led";

static int s_gpio = -1;
static esp_timer_handle_t s_timer;
static bool s_level;

static void blink_cb(void *arg)
{
    (void)arg;
    s_level = !s_level;
    gpio_set_level(s_gpio, s_level);
}

/* Half period of each blinking pattern; 0 means a steady level. */
static uint64_t half_period_us(status_led_state_t state)
{
    switch (state) {
    case STATUS_LED_IDLE:         return 500000;
    case STATUS_LED_PROVISIONING: return 125000;
    case STATUS_LED_OTA:          return 50000;
    case STATUS_LED_ERROR:        return 25000;
    default:                      return 0;
    }
}

esp_err_t status_led_init(int gpio)
{
    const gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << gpio,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&cfg), TAG, "gpio_config failed");

    const esp_timer_create_args_t args = {
        .callback = blink_cb,
        .name = "status_led",
    };
    ESP_RETURN_ON_ERROR(esp_timer_create(&args, &s_timer), TAG, "esp_timer_create failed");

    s_gpio = gpio;
    status_led_set(STATUS_LED_IDLE);
    return ESP_OK;
}

void status_led_set(status_led_state_t state)
{
    if (s_gpio < 0) {
        return;
    }
    esp_timer_stop(s_timer); /* ESP_ERR_INVALID_STATE if not running: harmless */

    const uint64_t half = half_period_us(state);
    if (half) {
        esp_timer_start_periodic(s_timer, half);
    } else {
        s_level = (state == STATUS_LED_CONNECTED);
        gpio_set_level(s_gpio, s_level);
    }
}
