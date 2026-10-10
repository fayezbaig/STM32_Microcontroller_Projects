/*
 * gpio.c - bare-metal GPIO driver for STM32F401/F411
 */
#include "gpio.h"

void gpio_clock_enable(GPIO_TypeDef *port)
{
    if (port == GPIOA)      RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    else if (port == GPIOB) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    else if (port == GPIOC) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    /* Dummy read: ensures the clock is running before the port is touched */
    (void)RCC->AHB1ENR;
}

void gpio_config(GPIO_TypeDef *port, uint8_t pin, gpio_mode_t mode,
                 gpio_otype_t otype, gpio_speed_t speed,
                 gpio_pull_t pull, uint8_t af)
{
    const uint32_t pos2 = (uint32_t)pin * 2U;

    /* Set the AF number first so the pin never glitches to a wrong AF */
    if (mode == GPIO_MODE_AF) {
        const uint32_t idx  = pin >> 3;              /* AFR[0]: pins 0-7, AFR[1]: 8-15 */
        const uint32_t pos4 = (uint32_t)(pin & 7U) * 4U;
        port->AFR[idx] = (port->AFR[idx] & ~(0xFUL << pos4)) | ((uint32_t)(af & 0xFU) << pos4);
    }

    port->OTYPER  = (port->OTYPER  & ~(1UL << pin))   | ((uint32_t)otype << pin);
    port->OSPEEDR = (port->OSPEEDR & ~(3UL << pos2))  | ((uint32_t)speed << pos2);
    port->PUPDR   = (port->PUPDR   & ~(3UL << pos2))  | ((uint32_t)pull  << pos2);
    port->MODER   = (port->MODER   & ~(3UL << pos2))  | ((uint32_t)mode  << pos2);
}

void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t level)
{
    /* BSRR: lower half sets, upper half resets - atomic, no read-modify-write */
    port->BSRR = level ? (1UL << pin) : (1UL << (pin + 16U));
}

void gpio_toggle(GPIO_TypeDef *port, uint8_t pin)
{
    port->ODR ^= (1UL << pin);
}

uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin)
{
    return (uint8_t)((port->IDR >> pin) & 1U);
}

/* ---------------- Board LED (PC13, active LOW) ---------------- */

void led_init(void)
{
    gpio_clock_enable(LED_PORT);
    gpio_config(LED_PORT, LED_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PUSHPULL,
                GPIO_SPEED_LOW, GPIO_PULL_NONE, 0);
    led_set(0);
}

void led_set(uint8_t on)
{
    gpio_write(LED_PORT, LED_PIN, (uint8_t)(on ? 0U : 1U));   /* active LOW */
}

void led_toggle(void)
{
    gpio_toggle(LED_PORT, LED_PIN);
}
