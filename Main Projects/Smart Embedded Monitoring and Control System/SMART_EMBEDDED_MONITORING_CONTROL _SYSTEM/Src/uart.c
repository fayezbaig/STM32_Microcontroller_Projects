/*
 * uart.c - bare-metal, polled USART2 driver
 */
#include "uart.h"
#include "gpio.h"
#include "stm32f4_reg.h"

void uart2_init(uint32_t baud)
{
    gpio_clock_enable(GPIOA);
    gpio_config(GPIOA, 2, GPIO_MODE_AF, GPIO_OTYPE_PUSHPULL, GPIO_SPEED_HIGH,
                GPIO_PULL_NONE, GPIO_AF7_USART2);            /* TX */
    gpio_config(GPIOA, 3, GPIO_MODE_AF, GPIO_OTYPE_PUSHPULL, GPIO_SPEED_HIGH,
                GPIO_PULL_UP,   GPIO_AF7_USART2);            /* RX */

    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    USART2->CR1 = 0;
    USART2->CR2 = 0;
    USART2->CR3 = 0;

    /* OVER8 = 0: BRR = pclk / baud (rounded).
     * 16 MHz / 9600 -> 1667 (0x683), same value as the original code */
    USART2->BRR = (PCLK1_HZ + (baud / 2U)) / baud;

    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void uart2_putc(char c)
{
    while (!(USART2->SR & USART_SR_TXE)) { }
    USART2->DR = (uint8_t)c;
}

void uart2_puts(const char *s)
{
    while (*s) {
        uart2_putc(*s++);
    }
}

void uart2_put_uint(uint32_t value, uint8_t width)
{
    char buf[10];
    uint8_t n = 0;

    do {
        buf[n++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value);

    while (width > n) {            /* left padding with spaces */
        uart2_putc(' ');
        width--;
    }
    while (n) {
        uart2_putc(buf[--n]);
    }
}
