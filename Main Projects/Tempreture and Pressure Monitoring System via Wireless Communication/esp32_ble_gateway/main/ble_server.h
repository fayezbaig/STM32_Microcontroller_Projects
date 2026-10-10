/*
 * ble_server.h - BLE GATT server (ESP-IDF, Bluedroid)
 *
 * Advertises as "BMP280-Room" with the standard Bluetooth SIG
 * Environmental Sensing Service, so generic apps (e.g. nRF Connect)
 * decode the values without a custom app:
 *
 *   Service 0x181A  Environmental Sensing
 *     0x2A6E  Temperature  sint16, 0.01 degC   [Read, Notify]
 *     0x2A6D  Pressure     uint32, 0.1 Pa      [Read, Notify]
 */
#ifndef BLE_SERVER_H
#define BLE_SERVER_H

#include <stdint.h>
#include "esp_err.h"

#define BLE_DEVICE_NAME   "BMP280-Room"

esp_err_t ble_server_init(void);

/* Store new values and notify a connected phone (if it subscribed). */
void ble_server_update(int32_t temp_centi, uint32_t press_pa);

#endif /* BLE_SERVER_H */
