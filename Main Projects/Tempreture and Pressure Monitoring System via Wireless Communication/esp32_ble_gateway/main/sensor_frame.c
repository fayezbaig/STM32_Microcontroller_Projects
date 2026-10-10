/*
 * sensor_frame.c - parser for the frame sent by the STM32 over UART
 */
#include "sensor_frame.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

static int hex_val(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

bool sensor_frame_parse(const char *line, sensor_frame_t *out)
{
    if (line == NULL || out == NULL || line[0] != '$') return false;

    const char *star = strchr(line, '*');
    if (star == NULL) return false;

    /* --- checksum --- */
    uint8_t cs = 0;
    for (const char *p = line + 1; p < star; p++) cs ^= (uint8_t)*p;

    const int hi = hex_val(star[1]);
    const int lo = (hi >= 0) ? hex_val(star[2]) : -1;
    if (hi < 0 || lo < 0) return false;
    if (star[3] != '\0' && star[3] != '\r' && star[3] != '\n') return false;
    if (cs != (uint8_t)((hi << 4) | lo)) return false;

    /* --- payload: BMP,<int>,<uint> --- */
    if (strncmp(line + 1, "BMP,", 4) != 0) return false;

    const char *p = line + 5;
    char *end = NULL;

    errno = 0;
    long t = strtol(p, &end, 10);
    if (errno || end == p || *end != ',') return false;

    p = end + 1;
    errno = 0;
    unsigned long pr = strtoul(p, &end, 10);
    if (errno || end == p || end != star || *p == '-') return false;

    /* plausibility: BMP280 range is -40..85 degC, 300..1100 hPa */
    if (t < -4000 || t > 8500) return false;
    if (pr < 30000UL || pr > 110000UL) return false;

    out->temp_centi = (int32_t)t;
    out->press_pa   = (uint32_t)pr;
    return true;
}
