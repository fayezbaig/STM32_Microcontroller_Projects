/*
 * uart.h - bare-metal, polled USART2 driver (TX only used)
 *
 *   PA2 = USART2_TX -> USB-TTL adapter RX
 *   PA3 = USART2_RX <- USB-TTL adapter TX
 *   9600 8N1
 */
#ifndef UART_H
#define UART_H

#include <stdint.h>

#define UART_BAUD   9600UL

void uart2_init(uint32_t baud);

void uart2_putc(char c);
void uart2_puts(const char *s);

/* Unsigned integer in decimal, right-aligned to 'width' characters
 * (width = 0 -> no padding). */
void uart2_put_uint(uint32_t value, uint8_t width);

#endif /* UART_H */
