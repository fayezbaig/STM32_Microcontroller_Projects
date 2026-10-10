/*
 * frame.h - number formatting + the STM32 -> ESP32 UART frame
 *
 * No printf/newlib needed (keeps the bare-metal build small).
 *
 * ESP32 frame format (ASCII, one line per measurement):
 *
 *     $BMP,<temp_centi>,<press_pa>*<CS>\r\n
 *     e.g.  $BMP,2345,101325*5B\r\n      -> 23.45 degC, 1013.25 hPa
 *
 *   CS = XOR of every character between '$' and '*', as 2 hex digits.
 *   The ESP32 side (sensor_frame.c) parses exactly this format.
 */
#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>

/* value = 2345, decimals = 2 -> "23.45". Returns string length. */
uint8_t fmt_fixed(char *buf, int32_t value, uint8_t decimals);

/* Unsigned/signed integer to decimal string. Return string length. */
uint8_t fmt_uint(char *buf, uint32_t value);
uint8_t fmt_int(char *buf, int32_t value);

/* Build the machine-readable frame for the ESP32. buf >= 32 bytes. */
uint8_t frame_build_esp(char *buf, int32_t temp_centi, uint32_t press_pa);

/* Build the human-readable line for the PC terminal. buf >= 48 bytes. */
uint8_t frame_build_pc(char *buf, int32_t temp_centi, uint32_t press_pa);

#endif /* FRAME_H */
