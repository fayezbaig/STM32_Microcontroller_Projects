/*
 * adc.h - ADC1 single-channel, software-triggered conversion
 *
 *   PA1 = ADC1_IN1 (potentiometer wiper; pot ends to 3V3 and GND)
 *   12-bit result: 0 .. 4095
 */
#ifndef ADC_H
#define ADC_H

#include <stdint.h>

#define ADC_MAX_VALUE   4095U

void     adc1_init(void);
uint16_t adc1_read(void);   /* blocking, ~6 us */

#endif /* ADC_H */
