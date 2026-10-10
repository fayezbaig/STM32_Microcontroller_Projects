/*
 * adc.c - ADC1 single-channel, software-triggered conversion
 */
#include "adc.h"
#include "gpio.h"
#include "stm32f4_reg.h"

#define ADC_CHANNEL     1U      /* PA1 */
#define ADC_SMP_84CYC   4U      /* SMPx = 100: 84 cycles - stable with a 10k pot */

void adc1_init(void)
{
    gpio_clock_enable(GPIOA);
    gpio_config(GPIOA, 1, GPIO_MODE_ANALOG, GPIO_OTYPE_PUSHPULL,
                GPIO_SPEED_LOW, GPIO_PULL_NONE, 0);

    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* ADC clock = PCLK2 / 2 = 8 MHz (reset default, max allowed 36 MHz) */
    ADC1->CR1 = 0;                                  /* 12-bit, no scan */
    ADC1->CR2 = 0;                                  /* single conversion, right aligned */

    /* Longer sample time for the selected channel (SMPR2: 3 bits per channel) */
    ADC1->SMPR2 = (ADC1->SMPR2 & ~(7UL << (ADC_CHANNEL * 3U)))
                | (ADC_SMP_84CYC << (ADC_CHANNEL * 3U));

    ADC1->SQR1 = 0;                                 /* L = 0 -> 1 conversion */
    ADC1->SQR3 = ADC_CHANNEL;                       /* 1st conversion = channel 1 */

    ADC1->CR2 |= ADC_CR2_ADON;

    /* tSTAB: ADC needs ~3 us after power-up before the first conversion */
    for (volatile uint32_t i = 0; i < 100U; i++) { }
}

uint16_t adc1_read(void)
{
    ADC1->CR2 |= ADC_CR2_SWSTART;
    while (!(ADC1->SR & ADC_SR_EOC)) { }
    return (uint16_t)(ADC1->DR & 0x0FFFU);          /* reading DR clears EOC */
}
