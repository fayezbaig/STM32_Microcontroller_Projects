/*
 * main.c - Room temperature monitor, STM32F401/F411 "Black Pill", bare metal
 *
 *   BMP280 --I2C1--> STM32 --USART1--> PC      (human-readable text)
 *                          --USART2--> ESP32   ($BMP frame) --BLE--> phone
 *                          --I2C1----> SSD1306 OLED
 *
 * Wiring (3.3 V everywhere, common GND):
 *   PB6  I2C1_SCL  -> BMP280 SCL, OLED SCL
 *   PB7  I2C1_SDA  -> BMP280 SDA, OLED SDA
 *   PA9  USART1_TX -> USB-TTL RX   (PC)
 *   PA10 USART1_RX <- USB-TTL TX   (optional)
 *   PA2  USART2_TX -> ESP32 GPIO16 (RX2)
 *   PA3  USART2_RX <- ESP32 GPIO17 (TX2, optional)
 *   PC13 on-board LED (blinks once per sample)
 */
#include <stdint.h>
#include "stm32f4_reg.h"
#include "systick.h"
#include "gpio.h"
#include "i2c.h"
#include "uart.h"
#include "bmp280.h"
#include "oled.h"
#include "frame.h"

#define SAMPLE_PERIOD_MS     1000U   /* one reading per second        */
#define SENSOR_RETRY_MS      2000U   /* re-try init if sensor missing */
#define MAX_READ_FAILURES    3U      /* then re-initialise the sensor */

static uint8_t s_oled_ok;

/* ------------------------------------------------------------------ */
/* OLED screens                                                        */
/* ------------------------------------------------------------------ */

static void draw_header(void)
{
    const char *title = "Room Monitor";
    oled_draw_string((uint8_t)((OLED_WIDTH - oled_text_width(title, 1)) / 2U), 0, title, 1);
    oled_hline(0, 10, OLED_WIDTH);
}

static void screen_message(const char *line1, const char *line2)
{
    if (!s_oled_ok) return;
    oled_clear();
    draw_header();
    oled_draw_string(0, 24, line1, 1);
    oled_draw_string(0, 36, line2, 1);
    oled_update();
}

static void screen_measurement(const bmp280_data_t *d)
{
    char buf[16];

    if (!s_oled_ok) return;
    oled_clear();
    draw_header();

    /* Big temperature, 2 decimals, scale 3 (18x24 px per char) */
    fmt_fixed(buf, d->temp_centi, 2);
    const uint8_t w = oled_text_width(buf, 3);
    const uint8_t x = (uint8_t)((OLED_WIDTH - (w + 14U)) / 2U);
    oled_draw_string(x, 16, buf, 3);
    oled_draw_string((uint8_t)(x + w + 2U), 16, OLED_DEG "C", 1);

    /* Pressure in hPa on the bottom line */
    char line[24] = "P: ";
    uint8_t n = 3;
    n = (uint8_t)(n + fmt_fixed(line + n, (int32_t)d->press_pa, 2));
    line[n++] = ' '; line[n++] = 'h'; line[n++] = 'P'; line[n++] = 'a'; line[n] = '\0';
    oled_draw_string(0, 50, line, 1);

    oled_update();
}

/* ------------------------------------------------------------------ */
/* Sensor helpers                                                      */
/* ------------------------------------------------------------------ */

static uint8_t sensor_start(void)
{
    bmp280_status_t st = bmp280_init();

    switch (st) {
    case BMP280_OK:
        uart_puts(UART_PC, "BMP280 OK (normal mode, 1 s standby)\r\n");
        screen_message("BMP280 found", "Waiting for data...");
        return 1;
    case BMP280_ERR_CHIP_ID:
        uart_puts(UART_PC, "ERROR: device at 0x76 is not a BMP280 (BME280? wrong ID)\r\n");
        screen_message("Wrong chip ID", "Is it a BMP280?");
        return 0;
    default:
        uart_puts(UART_PC, "ERROR: BMP280 not responding on I2C (check wiring/addr)\r\n");
        screen_message("BMP280 not found", "Check SDA/SCL/3V3");
        return 0;
    }
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(void)
{
    char line[64];

    systick_init();
    led_init();
    uart_init(UART_PC, UART_BAUD);
    uart_init(UART_ESP32, UART_BAUD);
    i2c1_init();

    delay_ms(100);    /* give the OLED charge pump / BMP280 time after power-up */

    uart_puts(UART_PC, "\r\n=== STM32 Room Monitor (bare metal) ===\r\n");

    s_oled_ok = (oled_init() == I2C_OK);
    uart_puts(UART_PC, s_oled_ok ? "OLED OK\r\n"
                                 : "WARNING: OLED not found at 0x3C - continuing without display\r\n");

    uint8_t  sensor_ok = sensor_start();
    uint8_t  failures  = 0;
    uint32_t last      = millis();

    for (;;) {
        const uint32_t now = millis();

        if (!sensor_ok) {
            if ((uint32_t)(now - last) >= SENSOR_RETRY_MS) {
                last = now;
                sensor_ok = sensor_start();
                failures = 0;
            }
            continue;
        }

        if ((uint32_t)(now - last) < SAMPLE_PERIOD_MS) {
            continue;
        }
        last += SAMPLE_PERIOD_MS;           /* fixed rate, no drift */

        bmp280_data_t d;
        bmp280_status_t st = bmp280_read(&d);

        if (st == BMP280_ERR_NOT_READY) {
            continue;                       /* first conversion still running */
        }

        if (st != BMP280_OK) {
            uart_puts(UART_PC, "ERROR: BMP280 read failed\r\n");
            if (++failures >= MAX_READ_FAILURES) {
                i2c1_init();                /* recover a stuck bus, then retry */
                if (s_oled_ok) s_oled_ok = (oled_init() == I2C_OK);
                sensor_ok = sensor_start();
                failures = 0;
            }
            continue;
        }
        failures = 0;

        /* 1) PC: human-readable line */
        frame_build_pc(line, d.temp_centi, d.press_pa);
        uart_puts(UART_PC, line);

        /* 2) ESP32: checksummed frame -> BLE */
        frame_build_esp(line, d.temp_centi, d.press_pa);
        uart_puts(UART_ESP32, line);

        /* 3) OLED */
        screen_measurement(&d);

        led_toggle();
    }
}
