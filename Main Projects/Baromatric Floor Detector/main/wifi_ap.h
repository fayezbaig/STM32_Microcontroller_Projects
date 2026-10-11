/*
 * wifi_ap.h - ESP32 runs its own WiFi network (SoftAP)
 *
 * The phone connects directly to the ESP32, so the tracker works anywhere
 * in a building - no router or building WiFi needed.
 * Page: http://192.168.4.1
 */
#ifndef WIFI_AP_H
#define WIFI_AP_H

#include "esp_err.h"

esp_err_t wifi_ap_start(void);

#endif /* WIFI_AP_H */
