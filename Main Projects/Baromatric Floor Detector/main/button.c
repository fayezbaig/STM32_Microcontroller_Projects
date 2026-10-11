/*
 * button.c - BOOT button (GPIO0) sets the ground floor
 */
#include "button.h"

#include "driver/gpio.h"
#include "esp_attr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tracker.h"

static volatile TickType_t s_last_press;

static void IRAM_ATTR button_isr(void *arg)
{
    (void)arg;
    const TickType_t now = xTaskGetTickCountFromISR();
    if ((now - s_last_press) >= pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS)) {
        s_last_press = now;
        tracker_request_zero_from_isr();
    }
}

esp_err_t button_init(void)
{
    const gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << BUTTON_GPIO,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_NEGEDGE,          /* pressed = pulled to GND */
    };
    esp_err_t err = gpio_config(&cfg);
    if (err != ESP_OK) return err;

    err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;  /* already installed is OK */

    return gpio_isr_handler_add(BUTTON_GPIO, button_isr, NULL);
}
