#pragma once

#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Called from the presence_sensor task (not from the ISR) after debouncing.
 * @param present true when the sensor output is at its active level.
 */
typedef void (*presence_sensor_cb_t)(bool present, void *ctx);

/**
 * Configure @p gpio as an interrupt input for a digital presence sensor and
 * report debounced changes through @p cb.
 *
 * @param active_low true for active-low outputs (IR obstacle modules),
 *                   false for active-high outputs (HC-SR501 PIR). The internal
 *                   pull resistor is set to the inactive level, so a
 *                   disconnected sensor reads as "not present".
 */
esp_err_t presence_sensor_init(int gpio, bool active_low, presence_sensor_cb_t cb, void *ctx);

#ifdef __cplusplus
}
#endif
