#pragma once

#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Called from the ir_sensor task (not from the ISR) after debouncing.
 * @param obstacle true when an obstacle is detected (module output LOW).
 */
typedef void (*ir_sensor_cb_t)(bool obstacle, void *ctx);

/**
 * Configure @p gpio as an interrupt input for an IR obstacle-avoidance module
 * (active-low digital output) and report debounced changes through @p cb.
 */
esp_err_t ir_sensor_init(int gpio, ir_sensor_cb_t cb, void *ctx);

#ifdef __cplusplus
}
#endif
