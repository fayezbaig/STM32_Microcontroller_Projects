/*
 * i2c.h - bare-metal, polled I2C1 master driver for STM32F401/F411
 *
 * Pins : PB6 = SCL, PB7 = SDA (AF4, open-drain)
 * Speed: Fast mode (~380 kHz with PCLK1 = 16 MHz)
 *
 * Both the BMP280 (0x76) and the SSD1306 OLED (0x3C) share this bus.
 * Addresses are given 7-bit (NOT shifted).
 */
#ifndef I2C_H
#define I2C_H

#include <stdint.h>

typedef enum {
    I2C_OK = 0,
    I2C_ERR_TIMEOUT,   /* a flag never came - wiring / pull-ups / stuck bus */
    I2C_ERR_NACK,      /* no device answered the address or a data byte      */
    I2C_ERR_BUS        /* bus error or arbitration lost                      */
} i2c_status_t;

/* Initialise I2C1 incl. GPIO setup and a 9-clock bus recovery. */
void i2c1_init(void);

/* Returns I2C_OK if a device ACKs its address. */
i2c_status_t i2c_probe(uint8_t addr7);

/* Plain write: START, addr+W, data[0..len-1], STOP */
i2c_status_t i2c_write(uint8_t addr7, const uint8_t *data, uint16_t len);

/* Register write: START, addr+W, reg, data[0..len-1], STOP
 * (also used for the OLED, where 'reg' is the control byte 0x00/0x40) */
i2c_status_t i2c_write_reg(uint8_t addr7, uint8_t reg, const uint8_t *data, uint16_t len);

/* Register read: START, addr+W, reg, RESTART, addr+R, data[0..len-1], STOP */
i2c_status_t i2c_read_reg(uint8_t addr7, uint8_t reg, uint8_t *data, uint16_t len);

#endif /* I2C_H */
