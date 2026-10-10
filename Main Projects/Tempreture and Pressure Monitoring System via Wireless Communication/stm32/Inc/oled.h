/*
 * oled.h - SSD1306 128x64 OLED driver (I2C, frame-buffered)
 *
 * Address 0x3C (some modules: 0x3D - change OLED_I2C_ADDR).
 * Drawing functions only modify the RAM buffer; call oled_update()
 * to push the whole 1 KB frame to the display.
 */
#ifndef OLED_H
#define OLED_H

#include <stdint.h>
#include "i2c.h"

#define OLED_I2C_ADDR   0x3CU
#define OLED_WIDTH      128U
#define OLED_HEIGHT     64U

/* Character 0x7F is replaced by a degree sign in the built-in font */
#define OLED_DEG        "\x7F"

i2c_status_t oled_init(void);
i2c_status_t oled_update(void);

void oled_clear(void);
void oled_set_pixel(uint8_t x, uint8_t y, uint8_t on);
void oled_hline(uint8_t x, uint8_t y, uint8_t w);

/* 5x7 font, 6 px advance. scale = 1 (6x8 cell), 2 (12x16), 3 (18x24) ... */
void oled_draw_char(uint8_t x, uint8_t y, char c, uint8_t scale);
void oled_draw_string(uint8_t x, uint8_t y, const char *s, uint8_t scale);

/* Pixel width of a string at the given scale (for centring) */
uint8_t oled_text_width(const char *s, uint8_t scale);

#endif /* OLED_H */
