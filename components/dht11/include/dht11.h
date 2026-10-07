#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int temperature_c;  /**< degrees Celsius (DHT11 resolution: 1 °C) */
    int humidity_pct;   /**< relative humidity, % (resolution: 1 %)   */
} dht11_reading_t;

/** Prepare @p gpio (open-drain, external or internal pull-up) for the DHT11. */
esp_err_t dht11_init(int gpio);

/**
 * Trigger a measurement and decode the 40-bit frame.
 * @return ESP_OK, ESP_ERR_TIMEOUT (no response) or ESP_ERR_INVALID_CRC (bad checksum).
 */
esp_err_t dht11_read(dht11_reading_t *out);

#ifdef __cplusplus
}
#endif
