/*
 * button.h - push button on PA7 with EXTI interrupt + debounce
 *
 *   PA7 -> button -> GND  (internal pull-up, falling edge = press)
 */
#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

#define BUTTON_DEBOUNCE_MS   50U   /* ignore edges closer than this */

void button_init(void);

/* Returns 1 once for every valid press (and clears it), otherwise 0 */
uint8_t button_take_press(void);

#endif /* BUTTON_H */
