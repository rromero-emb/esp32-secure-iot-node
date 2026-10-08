#include "dht11.h"

#include <stddef.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "driver/rmt_rx.h"
#include "esp_attr.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

/*
 * DHT11 single-wire protocol (times in us):
 *   host:   pull the line low >= 18 ms, then release it
 *   sensor: ~20-40 high, 80 low, 80 high (response), then 40 bits MSB first,
 *           each 50 low + ~27 high ("0") or ~70 high ("1"), then 50 low and release.
 *
 * The start pulse is a GPIO write while the task sleeps; the reply is captured
 * by the RMT receiver. No busy-wait loops and no interrupts disabled.
 */

#define RMT_RESOLUTION_HZ 1000000 /* 1 tick = 1 us */
#define START_PULSE_MS    20
#define BIT_ONE_MIN_US    48      /* threshold between ~27 us ("0") and ~70 us ("1") */
#define IDLE_US           200     /* longest pulse is 80 us: this much silence ends the frame */
#define GLITCH_NS         1000
#define FRAME_BITS        40
#define RX_SYMBOLS        64      /* a frame is ~43 symbols; 64 = one RMT memory block */
#define RX_TIMEOUT_MS     50      /* a frame lasts ~5 ms */
#define MIN_INTERVAL_US   1000000 /* after power-up and between reads */

static const char *TAG = "dht11";

static int s_gpio = -1;
static rmt_channel_handle_t s_rx;
static QueueHandle_t s_done; /* carries the number of received symbols */
static rmt_symbol_word_t s_symbols[RX_SYMBOLS];
static int64_t s_last_us;

static bool IRAM_ATTR on_recv_done(rmt_channel_handle_t chan, const rmt_rx_done_event_data_t *ev,
                                   void *ctx)
{
    (void)chan;
    BaseType_t woken = pdFALSE;
    const size_t n = ev->num_symbols;
    xQueueSendFromISR((QueueHandle_t)ctx, &n, &woken);
    return woken == pdTRUE;
}

/*
 * The data bits are the last 40 high pulses of the capture: anything before
 * them (the host releasing the line, the sensor's 80 us response) is skipped,
 * and the idle level at the end is longer than IDLE_US.
 */
static esp_err_t decode(const rmt_symbol_word_t *sym, size_t n, uint8_t data[5])
{
    uint16_t highs[RX_SYMBOLS * 2];
    size_t count = 0;
    bool end = false;

    for (size_t i = 0; i < n && !end; i++) {
        const uint16_t dur[2] = {sym[i].duration0, sym[i].duration1};
        const uint16_t lvl[2] = {sym[i].level0, sym[i].level1};
        for (int k = 0; k < 2; k++) {
            if (dur[k] == 0) { /* end marker */
                end = true;
                break;
            }
            if (lvl[k] && dur[k] < IDLE_US) {
                highs[count++] = dur[k];
            }
        }
    }

    if (count == 0) {
        return ESP_ERR_TIMEOUT; /* line went high and stayed there: no sensor */
    }
    if (count < FRAME_BITS) {
        return ESP_ERR_INVALID_RESPONSE;
    }

    const uint16_t *bits = &highs[count - FRAME_BITS];
    for (int i = 0; i < 5; i++) {
        data[i] = 0;
        for (int b = 0; b < 8; b++) {
            data[i] = (uint8_t)(data[i] << 1) | (bits[i * 8 + b] >= BIT_ONE_MIN_US);
        }
    }
    if ((uint8_t)(data[0] + data[1] + data[2] + data[3]) != data[4]) {
        return ESP_ERR_INVALID_CRC;
    }
    return ESP_OK;
}

esp_err_t dht11_init(int gpio)
{
    ESP_RETURN_ON_FALSE(!s_rx, ESP_ERR_INVALID_STATE, TAG, "already initialised");

    esp_err_t ret = ESP_OK;
    s_done = xQueueCreate(1, sizeof(size_t));
    ESP_RETURN_ON_FALSE(s_done, ESP_ERR_NO_MEM, TAG, "queue alloc failed");

    const rmt_rx_channel_config_t rx_cfg = {
        .gpio_num = gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RMT_RESOLUTION_HZ,
        .mem_block_symbols = RX_SYMBOLS,
    };
    ESP_GOTO_ON_ERROR(rmt_new_rx_channel(&rx_cfg, &s_rx), fail, TAG, "rmt_new_rx_channel failed");

    const rmt_rx_event_callbacks_t cbs = {.on_recv_done = on_recv_done};
    ESP_GOTO_ON_ERROR(rmt_rx_register_event_callbacks(s_rx, &cbs, s_done), fail, TAG,
                      "callback registration failed");
    ESP_GOTO_ON_ERROR(rmt_enable(s_rx), fail, TAG, "rmt_enable failed");

    /*
     * The RX channel routed the pad to its input (with the pull-up on). Also
     * make it an open-drain output for the start pulse: writing 1 releases the
     * line, so the sensor can drive it.
     */
    gpio_set_level(gpio, 1);
    ESP_GOTO_ON_ERROR(gpio_set_direction(gpio, GPIO_MODE_INPUT_OUTPUT_OD), fail, TAG,
                      "gpio_set_direction failed");

    s_gpio = gpio;
    s_last_us = esp_timer_get_time();
    return ESP_OK;

fail:
    if (s_rx) {
        rmt_disable(s_rx);
        rmt_del_channel(s_rx);
        s_rx = NULL;
    }
    vQueueDelete(s_done);
    s_done = NULL;
    return ret;
}

esp_err_t dht11_read(dht11_reading_t *out)
{
    ESP_RETURN_ON_FALSE(out, ESP_ERR_INVALID_ARG, TAG, "out is NULL");
    ESP_RETURN_ON_FALSE(s_gpio >= 0, ESP_ERR_INVALID_STATE, TAG, "not initialised");

    /* The sensor ignores requests sooner than 1 s after power-up or the previous read. */
    const int64_t wait_us = s_last_us + MIN_INTERVAL_US - esp_timer_get_time();
    if (wait_us > 0) {
        vTaskDelay(pdMS_TO_TICKS(wait_us / 1000) + 1);
    }

    xQueueReset(s_done);
    gpio_set_level(s_gpio, 0);
    vTaskDelay(pdMS_TO_TICKS(START_PULSE_MS) + 1); /* +1 tick: vTaskDelay may return early by up to one tick */

    const rmt_receive_config_t rx_cfg = {
        .signal_range_min_ns = GLITCH_NS,
        .signal_range_max_ns = IDLE_US * 1000,
    };
    /* Arm the receiver while the line is still low, then release it. */
    const esp_err_t err = rmt_receive(s_rx, s_symbols, sizeof(s_symbols), &rx_cfg);
    gpio_set_level(s_gpio, 1);
    s_last_us = esp_timer_get_time();
    ESP_RETURN_ON_ERROR(err, TAG, "rmt_receive failed");

    size_t n;
    if (xQueueReceive(s_done, &n, pdMS_TO_TICKS(RX_TIMEOUT_MS)) != pdTRUE) {
        /* Abort the pending receive so the next read can arm the channel again. */
        rmt_disable(s_rx);
        rmt_enable(s_rx);
        return ESP_ERR_TIMEOUT;
    }

    uint8_t data[5];
    ESP_RETURN_ON_ERROR(decode(s_symbols, n, data), TAG, "bad frame (%u symbols)", (unsigned)n);

    out->humidity_pct = data[0];
    /* Newer DHT11 revisions flag sub-zero temperatures with bit 7 of the decimal byte. */
    out->temperature_c = (data[3] & 0x80) ? -(int)data[2] : (int)data[2];
    return ESP_OK;
}
