/*
 * gpio.h - bare-metal GPIO driver for STM32F401/F411
 */
#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include "stm32f4_reg.h"

/* MODER values */
typedef enum {
    GPIO_MODE_INPUT  = 0,
    GPIO_MODE_OUTPUT = 1,
    GPIO_MODE_AF     = 2,
    GPIO_MODE_ANALOG = 3
} gpio_mode_t;

/* OTYPER values */
typedef enum {
    GPIO_OTYPE_PUSHPULL  = 0,
    GPIO_OTYPE_OPENDRAIN = 1
} gpio_otype_t;

/* OSPEEDR values */
typedef enum {
    GPIO_SPEED_LOW    = 0,
    GPIO_SPEED_MEDIUM = 1,
    GPIO_SPEED_FAST   = 2,
    GPIO_SPEED_HIGH   = 3
} gpio_speed_t;

/* PUPDR values */
typedef enum {
    GPIO_PULL_NONE = 0,
    GPIO_PULL_UP   = 1,
    GPIO_PULL_DOWN = 2
} gpio_pull_t;

/* Alternate function numbers used in this project */
#define GPIO_AF4_I2C1     4U
#define GPIO_AF7_USART1   7U
#define GPIO_AF7_USART2   7U

/* On-board LED of the Black Pill: PC13, active LOW */
#define LED_PORT  GPIOC
#define LED_PIN   13U

/* Enable the AHB1 clock of a GPIO port (GPIOA, GPIOB or GPIOC). */
void gpio_clock_enable(GPIO_TypeDef *port);

/* Full pin configuration. 'af' is ignored unless mode == GPIO_MODE_AF. */
void gpio_config(GPIO_TypeDef *port, uint8_t pin, gpio_mode_t mode,
                 gpio_otype_t otype, gpio_speed_t speed,
                 gpio_pull_t pull, uint8_t af);

/* Convenience helpers */
void    gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t level);
void    gpio_toggle(GPIO_TypeDef *port, uint8_t pin);
uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin);

/* Board helpers */
void led_init(void);
void led_set(uint8_t on);
void led_toggle(void);

#endif /* GPIO_H */
