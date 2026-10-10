/*
 * main.c - Potentiometer-controlled LED dimmer, STM32F401/F411 Black Pill, bare metal
 *
 *   PA1  ADC1_IN1  <- potentiometer wiper (ends to 3V3 and GND)
 *   PA0  TIM2_CH1  -> 330 R -> LED -> GND   (1 kHz PWM)
 *   PA7  button    -> GND                    (toggles the mode)
 *   PA2  USART2_TX -> USB-TTL RX             (9600 8N1 status output)
 *   PC13 on-board LED: on while dimmer mode is active
 *
 * Mode OFF    : LED dark, the pot has no effect.
 * Mode DIMMER : the pot sets LED brightness 0..100 %.
 * Each button press switches between the two modes.
 */
#include <stdint.h>
#include "stm32f4_reg.h"
#include "systick.h"
#include "gpio.h"
#include "uart.h"
#include "adc.h"
#include "pwm.h"
#include "timer.h"
#include "button.h"

typedef enum {
    MODE_OFF = 0,
    MODE_DIMMER
} mode_t;

/* 0..4095 ADC -> 0..1000 duty, full range (original /4 clipped at 3996) */
static uint16_t adc_to_duty(uint16_t adc)
{
    return (uint16_t)(((uint32_t)adc * PWM_DUTY_MAX + ADC_MAX_VALUE / 2U) / ADC_MAX_VALUE);
}

static void print_mode(mode_t m)
{
    uart2_puts(m == MODE_DIMMER ? "\r\n>> Mode: DIMMER (pot controls LED)\r\n"
                                : "\r\n>> Mode: OFF\r\n");
}

static void print_report(mode_t m, uint16_t adc, uint16_t duty)
{
    uart2_puts("Mode: ");
    uart2_puts(m == MODE_DIMMER ? "DIMMER" : "OFF   ");
    uart2_puts(" | ADC: ");
    uart2_put_uint(adc, 4);
    uart2_puts(" | Duty: ");
    uart2_put_uint(duty / 10U, 3);          /* 0..1000 -> 0..100 % */
    uart2_puts(" %\r\n");
}

int main(void)
{
    systick_init();              /* 1 ms tick, needed by the button debounce */
    led_init();
    uart2_init(UART_BAUD);
    adc1_init();
    pwm_init();
    button_init();
    timer3_init(REPORT_RATE_HZ);

    uart2_puts("\r\n=== STM32 LED Dimmer (bare metal) ===\r\n");
    uart2_puts("Press the button on PA7 to switch mode.\r\n");

    mode_t   mode     = MODE_OFF;
    uint32_t adc_filt = adc1_read() * 4U;   /* simple low-pass, 4x scaled */

    print_mode(mode);

    for (;;) {
        /* 1) Button -> toggle mode */
        if (button_take_press()) {
            mode = (mode == MODE_OFF) ? MODE_DIMMER : MODE_OFF;
            led_set(mode == MODE_DIMMER);
            print_mode(mode);
        }

        /* 2) Read pot, smooth ADC noise: y = 3/4 y + 1/4 x  (kills LED flicker) */
        adc_filt = adc_filt - (adc_filt / 4U) + adc1_read();
        const uint16_t adc = (uint16_t)(adc_filt / 4U);

        /* 3) Update brightness */
        const uint16_t duty = (mode == MODE_DIMMER) ? adc_to_duty(adc) : 0U;
        pwm_set_duty(duty);

        /* 4) Periodic status report (TIM3 interrupt sets the flag) */
        if (timer3_take_flag()) {
            print_report(mode, adc, duty);
        }
    }
}
