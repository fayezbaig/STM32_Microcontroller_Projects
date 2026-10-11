/*
 * wifi_ap.c - ESP32 runs its own WiFi network (SoftAP)
 */
#include "wifi_ap.h"

#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "sdkconfig.h"

#define TAG "wifi"

static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg; (void)base;
    if (id == WIFI_EVENT_AP_STACONNECTED) {
        const wifi_event_ap_staconnected_t *e = data;
        ESP_LOGI(TAG, "phone connected (" MACSTR ")", MAC2STR(e->mac));
    } else if (id == WIFI_EVENT_AP_STADISCONNECTED) {
        const wifi_event_ap_stadisconnected_t *e = data;
        ESP_LOGI(TAG, "phone disconnected (" MACSTR ")", MAC2STR(e->mac));
    }
}

esp_err_t wifi_ap_start(void)
{
    esp_err_t err;
    if ((err = esp_netif_init()) != ESP_OK) return err;
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;
    esp_netif_create_default_wifi_ap();

    const wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    if ((err = esp_wifi_init(&init_cfg)) != ESP_OK) return err;
    if ((err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                          wifi_event_handler, NULL)) != ESP_OK) return err;

    wifi_config_t cfg = { 0 };
    strncpy((char *)cfg.ap.ssid, CONFIG_AP_SSID, sizeof cfg.ap.ssid);
    cfg.ap.ssid_len       = (uint8_t)strlen(CONFIG_AP_SSID);
    cfg.ap.channel        = 1;
    cfg.ap.max_connection = 4;

    if (strlen(CONFIG_AP_PASSWORD) >= 8) {
        strncpy((char *)cfg.ap.password, CONFIG_AP_PASSWORD, sizeof cfg.ap.password);
        cfg.ap.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        ESP_LOGW(TAG, "password shorter than 8 characters -> open network");
        cfg.ap.authmode = WIFI_AUTH_OPEN;
    }

    if ((err = esp_wifi_set_mode(WIFI_MODE_AP)) != ESP_OK) return err;
    if ((err = esp_wifi_set_config(WIFI_IF_AP, &cfg)) != ESP_OK) return err;
    if ((err = esp_wifi_start()) != ESP_OK) return err;

    ESP_LOGI(TAG, "WiFi \"%s\" started - open http://192.168.4.1 on your phone", CONFIG_AP_SSID);
    return ESP_OK;
}
