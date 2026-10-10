/*
 * uart_rx.h - receives $BMP frames from the STM32 on UART2
 *
 *   ESP32 GPIO16 (RX2) <- STM32 PA2 (USART2_TX)
 *   ESP32 GPIO17 (TX2) -> STM32 PA3 (USART2_RX)   (not used yet)
 *   115200 8N1, common GND
 */
#ifndef UART_RX_H
#define UART_RX_H

#include "esp_err.h"
#include "sensor_frame.h"

#define STM32_UART_NUM      UART_NUM_2
#define STM32_UART_RX_PIN   16
#define STM32_UART_TX_PIN   17
#define STM32_UART_BAUD     115200

/* Called from the UART task for every valid frame */
typedef void (*sensor_frame_cb_t)(const sensor_frame_t *frame);

esp_err_t uart_rx_start(sensor_frame_cb_t cb);

#endif /* UART_RX_H */
