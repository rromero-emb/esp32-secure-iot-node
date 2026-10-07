#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Device states shown on the single status LED. */
typedef enum {
    STATUS_LED_OFF,
    STATUS_LED_IDLE,          /**< slow blink (1 Hz)  */
    STATUS_LED_PROVISIONING,  /**< fast blink (4 Hz)  */
    STATUS_LED_CONNECTED,     /**< solid on           */
    STATUS_LED_OTA,           /**< very fast (10 Hz)  */
    STATUS_LED_ERROR,         /**< flicker (20 Hz)    */
} status_led_state_t;

/** Configure @p gpio as output and start in STATUS_LED_IDLE. */
esp_err_t status_led_init(int gpio);

/** Change the pattern. Safe to call from any task (not from ISRs). */
void status_led_set(status_led_state_t state);

#ifdef __cplusplus
}
#endif
