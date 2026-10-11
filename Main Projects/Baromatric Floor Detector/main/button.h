/*
 * button.h - BOOT button (GPIO0) sets the ground floor
 *
 * Uses a GPIO interrupt with a time-based debounce; the ISR only sends
 * a task notification, all real work happens in the tracker task.
 */
#ifndef BUTTON_H
#define BUTTON_H

#include "esp_err.h"

#define BUTTON_GPIO          0      /* BOOT button on ESP32 DevKit boards */
#define BUTTON_DEBOUNCE_MS   250

esp_err_t button_init(void);

#endif /* BUTTON_H */
