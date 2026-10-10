/*
 * systick.c - 1 ms time base using the Cortex-M SysTick timer
 */
#include "systick.h"
#include "stm32f4_reg.h"

static volatile uint32_t s_ticks;

void systick_init(void)
{
    SYSTICK->CTRL = 0;
    SYSTICK->LOAD = (SYSCLK_HZ / 1000UL) - 1UL;   /* 1 ms period */
    SYSTICK->VAL  = 0;
    SYSTICK->CTRL = SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_ENABLE;
}

/* Name matches the vector table in the standard CMSIS/CubeIDE startup file */
void SysTick_Handler(void)
{
    s_ticks++;
}

uint32_t millis(void)
{
    return s_ticks;
}

void delay_ms(uint32_t ms)
{
    const uint32_t start = s_ticks;
    while ((uint32_t)(s_ticks - start) < ms) {
        /* wait - unsigned subtraction handles the 49-day wrap-around */
    }
}
