#include "dht11.h"

/*
 * TODO (session 1): RMT-based driver.
 *  - TX: hold the line low >= 18 ms to start a measurement.
 *  - RX: capture the 83 pulses of the response with the RMT receiver,
 *    decode 40 bits (bit = 1 when the high pulse is > ~50 us) and check the
 *    checksum. No busy-wait loops and no interrupts disabled.
 */

esp_err_t dht11_init(int gpio)
{
    (void)gpio;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t dht11_read(dht11_reading_t *out)
{
    (void)out;
    return ESP_ERR_NOT_SUPPORTED;
}
