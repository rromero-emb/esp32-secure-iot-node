#include "presence_sensor.h"

#include <stddef.h>

#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define DEBOUNCE_MS 30

static const char *TAG = "presence_sensor";

static QueueHandle_t s_queue;
static int s_gpio;
static int s_active_level;
static presence_sensor_cb_t s_cb;
static void *s_ctx;

static void IRAM_ATTR gpio_isr(void *arg)
{
    (void)arg;
    BaseType_t woken = pdFALSE;
    const uint8_t edge = 1;
    xQueueSendFromISR(s_queue, &edge, &woken);
    if (woken) {
        portYIELD_FROM_ISR();
    }
}

static void sensor_task(void *arg)
{
    (void)arg;
    int last = gpio_get_level(s_gpio);
    uint8_t edge;

    for (;;) {
        xQueueReceive(s_queue, &edge, portMAX_DELAY);
        /* Let the line settle, then drop the edges queued meanwhile. */
        vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));
        xQueueReset(s_queue);

        const int level = gpio_get_level(s_gpio);
        if (level != last) {
            last = level;
            s_cb(level == s_active_level, s_ctx);
        }
    }
}

esp_err_t presence_sensor_init(int gpio, bool active_low, presence_sensor_cb_t cb, void *ctx)
{
    ESP_RETURN_ON_FALSE(cb, ESP_ERR_INVALID_ARG, TAG, "callback required");

    s_queue = xQueueCreate(8, sizeof(uint8_t));
    ESP_RETURN_ON_FALSE(s_queue, ESP_ERR_NO_MEM, TAG, "queue alloc failed");
    s_gpio = gpio;
    s_active_level = active_low ? 0 : 1;
    s_cb = cb;
    s_ctx = ctx;

    const gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << gpio,
        .mode = GPIO_MODE_INPUT,
        /* Pull towards the inactive level: a missing sensor reads "not present". */
        .pull_up_en = active_low ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = active_low ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&cfg), TAG, "gpio_config failed");

    esp_err_t err = gpio_install_isr_service(0);
    ESP_RETURN_ON_FALSE(err == ESP_OK || err == ESP_ERR_INVALID_STATE, err, TAG,
                        "isr service install failed");
    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(gpio, gpio_isr, NULL), TAG, "isr add failed");

    ESP_RETURN_ON_FALSE(xTaskCreate(sensor_task, "presence", 3072, NULL, 5, NULL) == pdPASS,
                        ESP_ERR_NO_MEM, TAG, "task create failed");
    return ESP_OK;
}
