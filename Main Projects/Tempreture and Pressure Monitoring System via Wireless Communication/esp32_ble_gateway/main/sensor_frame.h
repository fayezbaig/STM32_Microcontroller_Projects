/*
 * sensor_frame.h - parser for the frame sent by the STM32 over UART
 *
 *     $BMP,<temp_centi>,<press_pa>*<CS>\r\n
 *     e.g.  $BMP,2345,101325*5B
 *
 * CS = XOR of all characters between '$' and '*', two hex digits.
 * Pure C, no ESP-IDF dependency (unit-testable on a PC).
 */
#ifndef SENSOR_FRAME_H
#define SENSOR_FRAME_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int32_t  temp_centi;   /* 0.01 degC */
    uint32_t press_pa;     /* Pa        */
} sensor_frame_t;

/* 'line' is one received line WITHOUT the trailing \r\n.
 * Returns true and fills 'out' only if format AND checksum are valid. */
bool sensor_frame_parse(const char *line, sensor_frame_t *out);

#endif /* SENSOR_FRAME_H */
