/*
 * bmp280.h - BMP280 driver for ESP-IDF (new i2c_master API, IDF >= 5.2)
 *
 * Configured for altitude measurement: pressure oversampling x16,
 * temperature x2, IIR filter x4, normal mode, ~26 Hz output rate.
 * Pressure is returned with 1/256 Pa resolution (1 Pa ~ 8.4 cm height).
 */
#ifndef BMP280_H
#define BMP280_H

#include <stdint.h>
#include "esp_err.h"

typedef struct {
    float pressure_pa;     /* e.g. 98765.43 */
    float temperature_c;   /* e.g. 23.45    */
} bmp280_reading_t;

/* Creates the I2C bus, checks the chip ID, loads calibration, starts measuring */
esp_err_t bmp280_init(int sda_gpio, int scl_gpio, uint8_t i2c_addr);

esp_err_t bmp280_read(bmp280_reading_t *out);

#endif /* BMP280_H */
