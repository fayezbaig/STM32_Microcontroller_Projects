/*
 * pwm.h - LED brightness PWM on TIM2 channel 1
 *
 *   PA0 = TIM2_CH1 (AF1) -> resistor (e.g. 330 R) -> LED -> GND
 *   1 kHz, duty resolution 0.1 % (0 .. 1000)
 */
#ifndef PWM_H
#define PWM_H

#include <stdint.h>

#define PWM_FREQ_HZ      1000U
#define PWM_DUTY_MAX     1000U    /* 1000 = 100.0 % */

void pwm_init(void);

/* duty: 0 (off) .. PWM_DUTY_MAX (fully on) */
void pwm_set_duty(uint16_t duty);

#endif /* PWM_H */
