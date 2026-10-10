/*
 * timer.h - TIM3 periodic interrupt (replaces the old TIM2 update IRQ)
 *
 * Sets a flag at REPORT_RATE_HZ that main() uses to print over UART.
 * Separate from the PWM timer, so changing the PWM frequency no longer
 * changes how often data is printed.
 */
#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define REPORT_RATE_HZ   5U     /* UART report 5 times per second */

void timer3_init(uint32_t rate_hz);

/* Returns 1 once per period (and clears the flag), otherwise 0 */
uint8_t timer3_take_flag(void);

#endif /* TIMER_H */
