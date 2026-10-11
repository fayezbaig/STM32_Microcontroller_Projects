/*
 * bmp280.c - BMP280 driver for ESP-IDF (new i2c_master API, IDF >= 5.2)
 *
 * Compensation: integer formulas from the Bosch datasheet (section 8.2).
 */
#include "bmp280.h"

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TAG               "bmp280"
#define I2C_TIMEOUT_MS    50
#define I2C_SPEED_HZ      400000

#define REG_CALIB         0x88
#define REG_ID            0xD0
#define REG_RESET         0xE0
#define REG_STATUS        0xF3
#define REG_CTRL_MEAS     0xF4
#define REG_CONFIG        0xF5
#define REG_DATA          0xF7

#define CHIP_ID_BMP280    0x58
#define RESET_CMD         0xB6

/* ctrl_meas: osrs_t x2 (010) | osrs_p x16 (101) | normal mode (11) */
#define CTRL_MEAS_VALUE   ((2u << 5) | (5u << 2) | 3u)
/* config: t_sb 0.5 ms (000) | IIR filter x4 (010) */
#define CONFIG_VALUE      ((0u << 5) | (2u << 2))

typedef struct {
    uint16_t T1; int16_t T2, T3;
    uint16_t P1; int16_t P2, P3, P4, P5, P6, P7, P8, P9;
} calib_t;

static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_dev;
static calib_t                 s_cal;

/* ------------------------------------------------------------------ */
/* I2C helpers                                                         */
/* ------------------------------------------------------------------ */
static esp_err_t read_regs(uint8_t reg, uint8_t *buf, size_t len)
{
    return i2c_master_transmit_receive(s_dev, &reg, 1, buf, len, I2C_TIMEOUT_MS);
}

static esp_err_t write_reg(uint8_t reg, uint8_t val)
{
    const uint8_t buf[2] = { reg, val };
    return i2c_master_transmit(s_dev, buf, sizeof buf, I2C_TIMEOUT_MS);
}

/* ------------------------------------------------------------------ */
/* Compensation (datasheet)                                            */
/* ------------------------------------------------------------------ */
static int32_t compensate_t(int32_t adc_T, int32_t *t_fine)
{
    int32_t var1 = ((((adc_T >> 3) - ((int32_t)s_cal.T1 << 1))) * ((int32_t)s_cal.T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)s_cal.T1)) *
                      ((adc_T >> 4) - ((int32_t)s_cal.T1))) >> 12) * ((int32_t)s_cal.T3)) >> 14;
    *t_fine = var1 + var2;
    return (*t_fine * 5 + 128) >> 8;                 /* 0.01 degC */
}

/* Returns pressure in Q24.8 format (Pa * 256), 0 on error */
static uint32_t compensate_p_q8(int32_t adc_P, int32_t t_fine)
{
    int64_t var1 = ((int64_t)t_fine) - 128000;
    int64_t var2 = var1 * var1 * (int64_t)s_cal.P6;
    var2 = var2 + ((var1 * (int64_t)s_cal.P5) * ((int64_t)1 << 17));
    var2 = var2 + (((int64_t)s_cal.P4) * ((int64_t)1 << 35));
    var1 = ((var1 * var1 * (int64_t)s_cal.P3) >> 8) + ((var1 * (int64_t)s_cal.P2) * ((int64_t)1 << 12));
    var1 = ((((int64_t)1) << 47) + var1) * ((int64_t)s_cal.P1) >> 33;
    if (var1 == 0) return 0;

    int64_t p = 1048576 - adc_P;
    p = (((p * ((int64_t)1 << 31)) - var2) * 3125) / var1;
    var1 = (((int64_t)s_cal.P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)s_cal.P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)s_cal.P7) * ((int64_t)1 << 4));
    return (uint32_t)p;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */
esp_err_t bmp280_init(int sda_gpio, int scl_gpio, uint8_t i2c_addr)
{
    const i2c_master_bus_config_t bus_cfg = {
        .i2c_port          = I2C_NUM_0,
        .sda_io_num        = sda_gpio,
        .scl_io_num        = scl_gpio,
        .clk_source        = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,   /* modules usually have their own too */
    };
    esp_err_t err = i2c_new_master_bus(&bus_cfg, &s_bus);
    if (err != ESP_OK) return err;

    err = i2c_master_probe(s_bus, i2c_addr, I2C_TIMEOUT_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "no device at 0x%02X - check wiring / address (0x76 or 0x77)", i2c_addr);
        return err;
    }

    const i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = i2c_addr,
        .scl_speed_hz    = I2C_SPEED_HZ,
    };
    err = i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev);
    if (err != ESP_OK) return err;

    uint8_t id = 0;
    if ((err = read_regs(REG_ID, &id, 1)) != ESP_OK) return err;
    if (id != CHIP_ID_BMP280) {
        ESP_LOGE(TAG, "chip ID 0x%02X is not a BMP280 (0x58)%s", id,
                 id == 0x60 ? " - this is a BME280" : "");
        return ESP_ERR_NOT_FOUND;
    }

    if ((err = write_reg(REG_RESET, RESET_CMD)) != ESP_OK) return err;
    vTaskDelay(pdMS_TO_TICKS(10));
    for (int i = 0; i < 20; i++) {                       /* wait for NVM copy */
        uint8_t st = 0;
        if ((err = read_regs(REG_STATUS, &st, 1)) != ESP_OK) return err;
        if (!(st & 0x01)) break;
        vTaskDelay(pdMS_TO_TICKS(2));
    }

    uint8_t r[24];
    if ((err = read_regs(REG_CALIB, r, sizeof r)) != ESP_OK) return err;
    s_cal.T1 = (uint16_t)(r[0]  | (r[1]  << 8));
    s_cal.T2 = (int16_t) (r[2]  | (r[3]  << 8));
    s_cal.T3 = (int16_t) (r[4]  | (r[5]  << 8));
    s_cal.P1 = (uint16_t)(r[6]  | (r[7]  << 8));
    s_cal.P2 = (int16_t) (r[8]  | (r[9]  << 8));
    s_cal.P3 = (int16_t) (r[10] | (r[11] << 8));
    s_cal.P4 = (int16_t) (r[12] | (r[13] << 8));
    s_cal.P5 = (int16_t) (r[14] | (r[15] << 8));
    s_cal.P6 = (int16_t) (r[16] | (r[17] << 8));
    s_cal.P7 = (int16_t) (r[18] | (r[19] << 8));
    s_cal.P8 = (int16_t) (r[20] | (r[21] << 8));
    s_cal.P9 = (int16_t) (r[22] | (r[23] << 8));

    if ((err = write_reg(REG_CONFIG, CONFIG_VALUE)) != ESP_OK) return err;   /* in sleep mode */
    if ((err = write_reg(REG_CTRL_MEAS, CTRL_MEAS_VALUE)) != ESP_OK) return err;

    ESP_LOGI(TAG, "BMP280 ready at 0x%02X (SDA=%d, SCL=%d)", i2c_addr, sda_gpio, scl_gpio);
    return ESP_OK;
}

esp_err_t bmp280_read(bmp280_reading_t *out)
{
    uint8_t d[6];
    esp_err_t err = read_regs(REG_DATA, d, sizeof d);   /* burst: same conversion */
    if (err != ESP_OK) return err;

    const int32_t adc_P = ((int32_t)d[0] << 12) | ((int32_t)d[1] << 4) | (d[2] >> 4);
    const int32_t adc_T = ((int32_t)d[3] << 12) | ((int32_t)d[4] << 4) | (d[5] >> 4);
    if (adc_P == 0x80000 || adc_T == 0x80000) return ESP_ERR_INVALID_STATE;  /* not ready */

    int32_t t_fine;
    const int32_t  t_centi = compensate_t(adc_T, &t_fine);
    const uint32_t p_q8    = compensate_p_q8(adc_P, t_fine);
    if (p_q8 == 0) return ESP_ERR_INVALID_RESPONSE;

    out->temperature_c = (float)t_centi / 100.0f;
    out->pressure_pa   = (float)p_q8 / 256.0f;
    return ESP_OK;
}
