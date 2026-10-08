#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int temperature_c;  /**< degrees Celsius (DHT11 resolution: 1 °C) */
    int humidity_pct;   /**< relative humidity, % (resolution: 1 %)   */
} dht11_reading_t;

/**
 * Prepare @p gpio for the DHT11: RMT receiver plus open-drain output for the
 * start pulse. The 3-pin module has its own pull-up; the bare 4-pin sensor
 * needs 10 kOhm between DATA and 3V3.
 */
esp_err_t dht11_init(int gpio);

/**
 * Trigger a measurement and decode the 40-bit frame. Blocks ~25 ms, longer if
 * called less than 1 s after init or after the previous read (sensor limit).
 * @return ESP_OK, ESP_ERR_TIMEOUT (no response), ESP_ERR_INVALID_RESPONSE
 *         (incomplete frame) or ESP_ERR_INVALID_CRC (bad checksum).
 */
esp_err_t dht11_read(dht11_reading_t *out);

#ifdef __cplusplus
}
#endif
