/*
 * main.c - Barometric floor tracker (ESP32, ESP-IDF, FreeRTOS)
 *
 * BMP280 (I2C) -> Kalman filter -> floor + stairs/elevator detection
 *              -> live dashboard on a phone over the ESP32's own WiFi.
 */
#include "nvs_flash.h"
#include "esp_log.h"

#include "tracker.h"
#include "button.h"
#include "wifi_ap.h"
#include "web_server.h"

#define TAG "app"

void app_main(void)
{
    /* NVS is required by the WiFi driver */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(tracker_start());     /* sensor + tracker tasks */
    ESP_ERROR_CHECK(button_init());       /* BOOT button = set ground floor */
    ESP_ERROR_CHECK(wifi_ap_start());
    ESP_ERROR_CHECK(web_server_start());

    ESP_LOGI(TAG, "Floor tracker running. Stand still for 3 s: this spot becomes floor 0.");
}
