/*
 * systick.h - 1 ms time base using the Cortex-M SysTick timer
 */
#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

void     systick_init(void);          /* 1 kHz tick from SYSCLK      */
uint32_t millis(void);                /* ms since systick_init()     */
void     delay_ms(uint32_t ms);       /* blocking delay              */

#endif /* SYSTICK_H */
