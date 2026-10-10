/*
 * pwm.c - LED brightness PWM on TIM2 channel 1
 *
 * Original code ran TIM2 at 10 Hz (PSC 1599, ARR 999): that blinks
 * instead of dimming. Now: 16 MHz / 16 = 1 MHz tick, ARR 999 -> 1 kHz.
 */
#include "pwm.h"
#include "gpio.h"
#include "stm32f4_reg.h"

#define GPIO_AF1_TIM2    1U
#define PWM_TICK_HZ      (PWM_FREQ_HZ * PWM_DUTY_MAX)   /* 1 MHz */

void pwm_init(void)
{
    gpio_clock_enable(GPIOA);
    gpio_config(GPIOA, 0, GPIO_MODE_AF, GPIO_OTYPE_PUSHPULL,
                GPIO_SPEED_LOW, GPIO_PULL_NONE, GPIO_AF1_TIM2);

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->CR1  = 0;
    TIM2->PSC  = (TIMCLK1_HZ / PWM_TICK_HZ) - 1U;      /* 15  */
    TIM2->ARR  = PWM_DUTY_MAX - 1U;                    /* 999 */
    TIM2->CCR1 = 0;                                    /* start dark */

    /* Channel 1: PWM mode 1 (active while CNT < CCR1), preload enabled
     * so duty changes take effect cleanly at the next period */
    TIM2->CCMR1 = (TIM2->CCMR1 & ~(0xFFUL))
                | (TIM_OCMODE_PWM1 << TIM_CCMR1_OC1M_Pos)
                | TIM_CCMR1_OC1PE;
    TIM2->CCER |= TIM_CCER_CC1E;                       /* output enable, active high */

    TIM2->CR1 |= TIM_CR1_ARPE;
    TIM2->EGR  = TIM_EGR_UG;                           /* load PSC/ARR/CCR1 now */
    TIM2->CR1 |= TIM_CR1_CEN;
}

void pwm_set_duty(uint16_t duty)
{
    if (duty > PWM_DUTY_MAX) duty = PWM_DUTY_MAX;
    /* CCR1 = ARR + 1 (= 1000) gives a constant high output, i.e. 100 % */
    TIM2->CCR1 = duty;
}
