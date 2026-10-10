/*
 * uart_rx.c - receives $BMP frames from the STM32 on UART2
 */
#include "uart_rx.h"

#include <stdlib.h>
#include <string.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TAG              "uart_rx"
#define RX_BUF_SIZE      1024
#define LINE_MAX         64

static sensor_frame_cb_t s_cb;

static void uart_rx_task(void *arg)
{
    (void)arg;
    uint8_t chunk[64];
    char    line[LINE_MAX];
    size_t  len = 0;
    bool    overflow = false;

    for (;;) {
        const int n = uart_read_bytes(STM32_UART_NUM, chunk, sizeof chunk, pdMS_TO_TICKS(100));
        for (int i = 0; i < n; i++) {
            const char c = (char)chunk[i];

            if (c == '\r') continue;

            if (c == '\n') {
                line[len] = '\0';
                if (!overflow && len > 0) {
                    sensor_frame_t f;
                    if (sensor_frame_parse(line, &f)) {
                        const long t = labs((long)f.temp_centi);
                        ESP_LOGI(TAG, "T=%s%ld.%02ld C  P=%lu Pa",
                                 (f.temp_centi < 0) ? "-" : "",
                                 t / 100, t % 100,
                                 (unsigned long)f.press_pa);
                        if (s_cb) s_cb(&f);
                    } else {
                        ESP_LOGW(TAG, "bad frame: '%s'", line);
                    }
                }
                len = 0;
                overflow = false;
                continue;
            }

            if (c == '$') {           /* resynchronise on every frame start */
                len = 0;
                overflow = false;
            }

            if (len < LINE_MAX - 1) {
                line[len++] = c;
            } else {
                overflow = true;      /* drop this line, wait for next '\n' */
            }
        }
    }
}

esp_err_t uart_rx_start(sensor_frame_cb_t cb)
{
    s_cb = cb;

    const uart_config_t cfg = {
        .baud_rate  = STM32_UART_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    esp_err_t err = uart_driver_install(STM32_UART_NUM, RX_BUF_SIZE, 0, 0, NULL, 0);
    if (err != ESP_OK) return err;
    err = uart_param_config(STM32_UART_NUM, &cfg);
    if (err != ESP_OK) return err;
    err = uart_set_pin(STM32_UART_NUM, STM32_UART_TX_PIN, STM32_UART_RX_PIN,
                       UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK) return err;

    if (xTaskCreate(uart_rx_task, "uart_rx", 4096, NULL, 5, NULL) != pdPASS) {
        return ESP_ERR_NO_MEM;
    }
    ESP_LOGI(TAG, "listening on UART%d (RX=GPIO%d) @ %d baud",
             STM32_UART_NUM, STM32_UART_RX_PIN, STM32_UART_BAUD);
    return ESP_OK;
}
