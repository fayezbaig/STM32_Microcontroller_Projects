/*
 * uart.c - bare-metal, polled USART driver for STM32F401/F411
 */
#include "uart.h"
#include "gpio.h"

/* Configure the pins + clock for the given USART instance */
static void uart_hw_setup(USART_TypeDef *uart, uint32_t *pclk)
{
    if (uart == USART1) {
        gpio_clock_enable(GPIOA);
        gpio_config(GPIOA, 9,  GPIO_MODE_AF, GPIO_OTYPE_PUSHPULL, GPIO_SPEED_HIGH,
                    GPIO_PULL_NONE, GPIO_AF7_USART1);            /* TX */
        gpio_config(GPIOA, 10, GPIO_MODE_AF, GPIO_OTYPE_PUSHPULL, GPIO_SPEED_HIGH,
                    GPIO_PULL_UP,   GPIO_AF7_USART1);            /* RX */
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
        *pclk = PCLK2_HZ;
    } else if (uart == USART2) {
        gpio_clock_enable(GPIOA);
        gpio_config(GPIOA, 2, GPIO_MODE_AF, GPIO_OTYPE_PUSHPULL, GPIO_SPEED_HIGH,
                    GPIO_PULL_NONE, GPIO_AF7_USART2);            /* TX */
        gpio_config(GPIOA, 3, GPIO_MODE_AF, GPIO_OTYPE_PUSHPULL, GPIO_SPEED_HIGH,
                    GPIO_PULL_UP,   GPIO_AF7_USART2);            /* RX */
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
        *pclk = PCLK1_HZ;
    }
}

void uart_init(USART_TypeDef *uart, uint32_t baud)
{
    uint32_t pclk = 0;
    uart_hw_setup(uart, &pclk);
    if (pclk == 0U) return;                     /* unsupported instance */

    uart->CR1 = 0;                              /* disable while configuring */
    uart->CR2 = 0;                              /* 1 stop bit */
    uart->CR3 = 0;                              /* no flow control */

    /* OVER8 = 0: USARTDIV = pclk / (16 * baud); BRR holds USARTDIV * 16
     * (mantissa << 4 | fraction), i.e. simply pclk / baud, rounded.
     * 16 MHz / 115200 -> 139 (0x8B) -> 115108 baud, error -0.08 % */
    uart->BRR = (pclk + (baud / 2U)) / baud;

    uart->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void uart_putc(USART_TypeDef *uart, char c)
{
    while (!(uart->SR & USART_SR_TXE)) { }
    uart->DR = (uint8_t)c;
}

void uart_puts(USART_TypeDef *uart, const char *s)
{
    while (*s) {
        uart_putc(uart, *s++);
    }
}

void uart_write(USART_TypeDef *uart, const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        uart_putc(uart, (char)data[i]);
    }
}

void uart_flush(USART_TypeDef *uart)
{
    while (!(uart->SR & USART_SR_TC)) { }
}

uint8_t uart_getc_nb(USART_TypeDef *uart, char *c)
{
    if (uart->SR & USART_SR_RXNE) {
        *c = (char)(uart->DR & 0xFFU);
        return 1U;
    }
    return 0U;
}
