/*
 * bmp280.c - Bosch BMP280 temperature/pressure sensor driver (I2C)
 *
 * Integer-only compensation (no FPU / float needed), taken from the
 * BMP280 datasheet (BST-BMP280-DS001), section 8.2.
 */
#include "bmp280.h"

#ifndef BMP280_HOST_TEST
#include "systick.h"
#endif

/* ---------------- Register map ---------------- */
#define REG_CALIB_START   0x88U   /* 0x88..0x9F, 24 bytes */
#define REG_CHIP_ID       0xD0U
#define REG_RESET         0xE0U
#define REG_STATUS        0xF3U
#define REG_CTRL_MEAS     0xF4U
#define REG_CONFIG        0xF5U
#define REG_DATA_START    0xF7U   /* press_msb..temp_xlsb, 6 bytes */

#define RESET_CMD         0xB6U
#define STATUS_IM_UPDATE  0x01U

/* ctrl_meas: osrs_t[7:5] | osrs_p[4:2] | mode[1:0]
 *   osrs_t = x2  (010), osrs_p = x16 (101), mode = normal (11) */
#define CTRL_MEAS_VALUE   ((2U << 5) | (5U << 2) | 3U)     /* 0x57 */

/* config: t_sb[7:5] | filter[4:2] | spi3w_en[0]
 *   t_sb = 1000 ms (101), IIR filter = x4 (010) - smooth room readings */
#define CONFIG_VALUE      ((5U << 5) | (2U << 2))          /* 0xA8 */

/* Raw value the sensor returns when a channel is skipped / not measured */
#define ADC_SKIPPED       0x80000


/* ------------------------------------------------------------------ */
/* Compensation math (host-testable, no hardware access)               */
/* ------------------------------------------------------------------ */

void bmp280_parse_calib(const uint8_t r[24], bmp280_calib_t *c)
{
    /* All calibration words are little-endian */
    c->dig_T1 = (uint16_t)(r[0]  | (r[1]  << 8));
    c->dig_T2 = (int16_t) (r[2]  | (r[3]  << 8));
    c->dig_T3 = (int16_t) (r[4]  | (r[5]  << 8));
    c->dig_P1 = (uint16_t)(r[6]  | (r[7]  << 8));
    c->dig_P2 = (int16_t) (r[8]  | (r[9]  << 8));
    c->dig_P3 = (int16_t) (r[10] | (r[11] << 8));
    c->dig_P4 = (int16_t) (r[12] | (r[13] << 8));
    c->dig_P5 = (int16_t) (r[14] | (r[15] << 8));
    c->dig_P6 = (int16_t) (r[16] | (r[17] << 8));
    c->dig_P7 = (int16_t) (r[18] | (r[19] << 8));
    c->dig_P8 = (int16_t) (r[20] | (r[21] << 8));
    c->dig_P9 = (int16_t) (r[22] | (r[23] << 8));
}

/* Returns temperature in 0.01 degC. t_fine is needed for pressure. */
int32_t bmp280_compensate_t(const bmp280_calib_t *c, int32_t adc_T, int32_t *t_fine)
{
    int32_t var1, var2;

    var1 = ((((adc_T >> 3) - ((int32_t)c->dig_T1 << 1))) * ((int32_t)c->dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)c->dig_T1)) *
              ((adc_T >> 4) - ((int32_t)c->dig_T1))) >> 12) *
            ((int32_t)c->dig_T3)) >> 14;

    *t_fine = var1 + var2;
    return (*t_fine * 5 + 128) >> 8;
}

/* Returns pressure in Pa (64-bit datasheet variant, rounded from Q24.8). */
uint32_t bmp280_compensate_p(const bmp280_calib_t *c, int32_t adc_P, int32_t t_fine)
{
    int64_t var1, var2, p;

    /* Same arithmetic as the datasheet. Left shifts of possibly negative
     * values are written as multiplications (identical result, but no
     * undefined behaviour in C); right shifts stay arithmetic shifts. */
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)c->dig_P6;
    var2 = var2 + ((var1 * (int64_t)c->dig_P5) * ((int64_t)1 << 17));
    var2 = var2 + (((int64_t)c->dig_P4) * ((int64_t)1 << 35));
    var1 = ((var1 * var1 * (int64_t)c->dig_P3) >> 8) +
           ((var1 * (int64_t)c->dig_P2) * ((int64_t)1 << 12));
    var1 = ((((int64_t)1) << 47) + var1) * ((int64_t)c->dig_P1) >> 33;

    if (var1 == 0) {
        return 0;                       /* avoid division by zero */
    }

    p = 1048576 - adc_P;
    p = (((p * ((int64_t)1 << 31)) - var2) * 3125) / var1;
    var1 = (((int64_t)c->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)c->dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)c->dig_P7) * ((int64_t)1 << 4));

    /* p is now Q24.8 (Pa * 256); round to whole Pa */
    return (uint32_t)((p + 128) / 256);
}

/* ------------------------------------------------------------------ */
/* Hardware access                                                     */
/* ------------------------------------------------------------------ */
#ifndef BMP280_HOST_TEST

static bmp280_calib_t s_calib;
static uint8_t        s_initialised;

static i2c_status_t write_reg(uint8_t reg, uint8_t val)
{
    return i2c_write_reg(BMP280_I2C_ADDR, reg, &val, 1);
}

bmp280_status_t bmp280_init(void)
{
    uint8_t id = 0;
    uint8_t status = 0;
    uint8_t raw[24];

    s_initialised = 0;

    if (i2c_read_reg(BMP280_I2C_ADDR, REG_CHIP_ID, &id, 1) != I2C_OK) {
        return BMP280_ERR_I2C;
    }
    if (id != BMP280_CHIP_ID) {
        return BMP280_ERR_CHIP_ID;
    }

    /* Soft reset, then wait until the NVM calibration copy is finished */
    if (write_reg(REG_RESET, RESET_CMD) != I2C_OK) return BMP280_ERR_I2C;
    delay_ms(5);
    for (uint8_t i = 0; i < 20U; i++) {
        if (i2c_read_reg(BMP280_I2C_ADDR, REG_STATUS, &status, 1) != I2C_OK) {
            return BMP280_ERR_I2C;
        }
        if (!(status & STATUS_IM_UPDATE)) break;
        delay_ms(2);
    }

    if (i2c_read_reg(BMP280_I2C_ADDR, REG_CALIB_START, raw, sizeof raw) != I2C_OK) {
        return BMP280_ERR_I2C;
    }
    bmp280_parse_calib(raw, &s_calib);

    /* config must be written while in sleep mode (after reset we are) */
    if (write_reg(REG_CONFIG, CONFIG_VALUE)    != I2C_OK) return BMP280_ERR_I2C;
    if (write_reg(REG_CTRL_MEAS, CTRL_MEAS_VALUE) != I2C_OK) return BMP280_ERR_I2C;

    s_initialised = 1;
    return BMP280_OK;
}

bmp280_status_t bmp280_read(bmp280_data_t *out)
{
    uint8_t d[6];
    int32_t t_fine;

    if (!s_initialised) return BMP280_ERR_NOT_READY;

    /* Burst read guarantees press + temp come from the same conversion */
    if (i2c_read_reg(BMP280_I2C_ADDR, REG_DATA_START, d, sizeof d) != I2C_OK) {
        return BMP280_ERR_I2C;
    }

    const int32_t adc_P = ((int32_t)d[0] << 12) | ((int32_t)d[1] << 4) | (d[2] >> 4);
    const int32_t adc_T = ((int32_t)d[3] << 12) | ((int32_t)d[4] << 4) | (d[5] >> 4);

    if (adc_T == ADC_SKIPPED || adc_P == ADC_SKIPPED) {
        return BMP280_ERR_NOT_READY;            /* first conversion not done yet */
    }

    out->temp_centi = bmp280_compensate_t(&s_calib, adc_T, &t_fine);
    out->press_pa   = bmp280_compensate_p(&s_calib, adc_P, t_fine);
    return BMP280_OK;
}

#endif /* BMP280_HOST_TEST */
