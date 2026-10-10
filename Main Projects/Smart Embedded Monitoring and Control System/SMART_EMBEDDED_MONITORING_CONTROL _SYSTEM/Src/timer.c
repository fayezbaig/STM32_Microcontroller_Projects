/*
 * timer.c - TIM3 periodic interrupt
 */
#include "timer.h"
#include "stm32f4_reg.h"

#define TIM3_TICK_HZ   10000UL     /* 16 MHz / 1600 = 10 kHz counter */

static volatile uint8_t s_flag;

void timer3_init(uint32_t rate_hz)
{
    if (rate_hz == 0U) rate_hz = 1U;

    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    TIM3->CR1  = 0;
    TIM3->PSC  = (TIMCLK1_HZ / TIM3_TICK_HZ) - 1U;   /* 1599 */
    TIM3->ARR  = (TIM3_TICK_HZ / rate_hz) - 1U;      /* 1999 for 5 Hz (16-bit timer!) */
    TIM3->EGR  = TIM_EGR_UG;                         /* load PSC/ARR now ...  */
    TIM3->SR   = 0;                                  /* ... and drop the UIF that UG set */
    TIM3->DIER = TIM_DIER_UIE;
    TIM3->CR1  = TIM_CR1_CEN;

    nvic_enable_irq(TIM3_IRQn);
}

uint8_t timer3_take_flag(void)
{
    if (s_flag) {
        s_flag = 0;
        return 1U;
    }
    return 0U;
}

/* Overrides the weak handler in startup_stm32f4.c */
void TIM3_IRQHandler(void)
{
    if (TIM3->SR & TIM_SR_UIF) {
        /* rc_w0 register: write 0 only to UIF, 1 elsewhere (no read-modify-write,
         * which could accidentally clear a flag set in the meantime) */
        TIM3->SR = ~TIM_SR_UIF;
        s_flag = 1;
    }
}
