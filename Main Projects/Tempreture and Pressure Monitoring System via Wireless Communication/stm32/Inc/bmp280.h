/*
 * bmp280.h - Bosch BMP280 temperature/pressure sensor driver (I2C)
 *
 * SDO pin -> GND : address 0x76 (default here)
 * SDO pin -> VDD : address 0x77
 */
#ifndef BMP280_H
#define BMP280_H

#include <stdint.h>
#include "i2c.h"

#define BMP280_I2C_ADDR      0x76U
#define BMP280_CHIP_ID       0x58U   /* a BME280 would report 0x60 */

typedef struct {
    int32_t  temp_centi;   /* temperature in 0.01 degC, e.g. 2345 = 23.45 degC */
    uint32_t press_pa;     /* pressure in Pa,           e.g. 101325             */
} bmp280_data_t;

typedef enum {
    BMP280_OK = 0,
    BMP280_ERR_I2C,        /* bus/communication failure          */
    BMP280_ERR_CHIP_ID,    /* something answered, but not a BMP280 */
    BMP280_ERR_NOT_READY   /* no valid measurement yet           */
} bmp280_status_t;

/* Soft-reset, verify chip ID, read calibration, start normal mode. */
bmp280_status_t bmp280_init(void);

/* Read and compensate the latest measurement. */
bmp280_status_t bmp280_read(bmp280_data_t *out);

/* Pure compensation math (Bosch datasheet, section 8.2).
 * Exposed so it can be unit-tested on a PC. */
typedef struct {
    uint16_t dig_T1; int16_t dig_T2; int16_t dig_T3;
    uint16_t dig_P1; int16_t dig_P2; int16_t dig_P3;
    int16_t  dig_P4; int16_t dig_P5; int16_t dig_P6;
    int16_t  dig_P7; int16_t dig_P8; int16_t dig_P9;
} bmp280_calib_t;

void    bmp280_parse_calib(const uint8_t raw[24], bmp280_calib_t *c);
int32_t bmp280_compensate_t(const bmp280_calib_t *c, int32_t adc_T, int32_t *t_fine);
uint32_t bmp280_compensate_p(const bmp280_calib_t *c, int32_t adc_P, int32_t t_fine);

#endif /* BMP280_H */
