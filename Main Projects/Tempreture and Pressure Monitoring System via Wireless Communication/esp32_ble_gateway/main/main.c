/*
 * main.c - ESP32 BLE gateway (ESP-IDF + Bluedroid)
 *
 *   STM32 --UART2 ($BMP frames)--> ESP32 --BLE (ESS 0x181A)--> phone
 *
 * Phone: open nRF Connect -> scan -> connect to "BMP280-Room" ->
 *        Environmental Sensing -> Temperature -> tap the notify (arrows) icon.
 */
#include "nvs_flash.h"
#include "esp_log.h"

#include "ble_server.h"
#include "uart_rx.h"

#define TAG "app"

static void on_frame(const sensor_frame_t *f)
{
    ble_server_update(f->temp_centi, f->press_pa);
}

void app_main(void)
{
    /* Bluedroid stores bonding info in NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(ble_server_init());
    ESP_ERROR_CHECK(uart_rx_start(on_frame));

    ESP_LOGI(TAG, "gateway running");
}
