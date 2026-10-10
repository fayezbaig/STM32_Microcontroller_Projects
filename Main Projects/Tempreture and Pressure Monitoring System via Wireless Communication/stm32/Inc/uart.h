/*
 * uart.h - bare-metal, polled USART driver for STM32F401/F411
 *
 *   USART1 : PA9 = TX, PA10 = RX  -> PC (via USB-TTL adapter)
 *   USART2 : PA2 = TX, PA3  = RX  -> ESP32 (ESP32 GPIO16 = RX2)
 *
 *   8 data bits, no parity, 1 stop bit (8N1)
 */
#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "stm32f4_reg.h"

#define UART_PC      USART1
#define UART_ESP32   USART2
#define UART_BAUD    115200UL

void uart_init(USART_TypeDef *uart, uint32_t baud);

void uart_putc(USART_TypeDef *uart, char c);
void uart_puts(USART_TypeDef *uart, const char *s);
void uart_write(USART_TypeDef *uart, const uint8_t *data, uint16_t len);

/* Block until the last byte has physically left the shift register */
void uart_flush(USART_TypeDef *uart);

/* Non-blocking receive: returns 1 and stores the byte if one is available */
uint8_t uart_getc_nb(USART_TypeDef *uart, char *c);

#endif /* UART_H */
