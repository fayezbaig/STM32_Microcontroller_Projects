/*
 * frame.c - number formatting + the STM32 -> ESP32 UART frame
 */
#include "frame.h"

static const char HEX[] = "0123456789ABCDEF";

uint8_t fmt_uint(char *buf, uint32_t value)
{
    char tmp[10];
    uint8_t n = 0;

    do {
        tmp[n++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value);

    for (uint8_t i = 0; i < n; i++) {
        buf[i] = tmp[n - 1U - i];
    }
    buf[n] = '\0';
    return n;
}

uint8_t fmt_int(char *buf, int32_t value)
{
    if (value < 0) {
        buf[0] = '-';
        /* cast via int64 so INT32_MIN does not overflow */
        return (uint8_t)(1U + fmt_uint(buf + 1, (uint32_t)(-(int64_t)value)));
    }
    return fmt_uint(buf, (uint32_t)value);
}

uint8_t fmt_fixed(char *buf, int32_t value, uint8_t decimals)
{
    uint32_t div = 1;
    for (uint8_t i = 0; i < decimals; i++) div *= 10U;

    uint8_t n = 0;
    uint32_t mag;
    if (value < 0) {
        buf[n++] = '-';
        mag = (uint32_t)(-(int64_t)value);
    } else {
        mag = (uint32_t)value;
    }

    n = (uint8_t)(n + fmt_uint(buf + n, mag / div));

    if (decimals) {
        uint32_t frac = mag % div;
        buf[n++] = '.';
        for (uint8_t i = decimals; i > 0U; i--) {          /* zero-padded */
            uint32_t p = 1;
            for (uint8_t k = 1; k < i; k++) p *= 10U;
            buf[n++] = (char)('0' + (frac / p) % 10U);
        }
    }
    buf[n] = '\0';
    return n;
}

static uint8_t append(char *buf, uint8_t n, const char *s)
{
    while (*s) buf[n++] = *s++;
    buf[n] = '\0';
    return n;
}

uint8_t frame_build_esp(char *buf, int32_t temp_centi, uint32_t press_pa)
{
    uint8_t n = 0;

    n = append(buf, n, "$BMP,");
    n = (uint8_t)(n + fmt_int(buf + n, temp_centi));
    buf[n++] = ',';
    n = (uint8_t)(n + fmt_uint(buf + n, press_pa));

    uint8_t cs = 0;
    for (uint8_t i = 1; i < n; i++) {    /* skip the leading '$' */
        cs ^= (uint8_t)buf[i];
    }

    buf[n++] = '*';
    buf[n++] = HEX[cs >> 4];
    buf[n++] = HEX[cs & 0x0FU];
    buf[n++] = '\r';
    buf[n++] = '\n';
    buf[n]   = '\0';
    return n;
}

uint8_t frame_build_pc(char *buf, int32_t temp_centi, uint32_t press_pa)
{
    uint8_t n = 0;

    n = append(buf, n, "Temperature: ");
    n = (uint8_t)(n + fmt_fixed(buf + n, temp_centi, 2));
    n = append(buf, n, " C   Pressure: ");
    /* Pa -> hPa with 2 decimals: 101325 Pa = 1013.25 hPa */
    n = (uint8_t)(n + fmt_fixed(buf + n, (int32_t)press_pa, 2));
    n = append(buf, n, " hPa\r\n");
    return n;
}
