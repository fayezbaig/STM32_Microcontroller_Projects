
# Temperature & Pressure Monitoring System via Wireless Communication Protocol and BMP280
=======
# Room Temperature & Pressure Monitoring System via Wireless Communication Protocol and BMP280

*STM32F401/F411 + BMP280 + SSD1306 OLED + ESP32 BLE*

This project measures room temperature and air pressure and shows them in three places: on a small
OLED screen, on a PC, and wirelessly on a phone.

Once per second, an STM32 "Black Pill" reads a BMP280 sensor over I2C. It then:
- draws the temperature in large digits on the OLED, with the pressure below it,
- prints a readable line to the PC over UART,
- sends a checksummed data frame over a second UART to an ESP32.

The ESP32 passes the values on over Bluetooth Low Energy using the standard Environmental Sensing
service. Any BLE app, such as nRF Connect, shows the temperature directly, with no custom phone app needed.

The STM32 firmware is fully bare metal: register-level drivers, no HAL, no CMSIS and no printf, and
integer-only math. The ESP32 uses ESP-IDF with the Bluedroid stack, because its Bluetooth radio can
only be driven through Espressif's own libraries.
>>>>>>> c09aeb0a21e05e7893cb62bf871818fc97492bfc

```
BMP280 --I2C1--> STM32F4 (bare metal) --USART1--> PC terminal (text)
                                      --USART2--> ESP32 --BLE--> phone
                                      --I2C1----> SSD1306 OLED 128x64
```

## Files

### stm32/ (bare metal, register level, no HAL/CMSIS, no printf)
| File | Purpose |
|---|---|
| `Inc/stm32f4_reg.h` | Register map for RCC, GPIO, I2C1, USART1/2, SysTick |
| `Inc/gpio.h`, `Src/gpio.c` | Pin configuration, on-board LED (PC13) |
| `Inc/systick.h`, `Src/systick.c` | 1 ms tick, `delay_ms()`, `millis()` |
| `Inc/i2c.h`, `Src/i2c.c` | Polled I2C1 master, ~380 kHz, timeouts, bus recovery |
| `Inc/uart.h`, `Src/uart.c` | Polled USART1 (PC) and USART2 (ESP32), 115200 8N1 |
| `Inc/bmp280.h`, `Src/bmp280.c` | BMP280 driver with integer compensation from the datasheet |
| `Inc/oled.h`, `Src/oled.c` | SSD1306 frame-buffer driver with 5x7 font (scalable) |
| `Inc/frame.h`, `Src/frame.c` | Number formatting and the `$BMP` frame sent to the ESP32 |
| `Src/main.c` | Application: 1 Hz sampling, PC + ESP32 + OLED output, error recovery |

### esp32_ble_gateway/ (ESP-IDF v5.x, Bluedroid)
| File | Purpose |
|---|---|
| `main/main.c` | NVS init, starts BLE and the UART receiver |
| `main/uart_rx.c/.h` | UART2 line receiver (GPIO16 RX) |
| `main/sensor_frame.c/.h` | Parses and checksum-verifies `$BMP` frames |
| `main/ble_server.c/.h` | GATT server: Environmental Sensing Service 0x181A |
| `CMakeLists.txt`, `main/CMakeLists.txt`, `sdkconfig.defaults` | ESP-IDF build files (BLE-only Bluedroid) |

## Wiring (all 3.3 V, one common GND)

| STM32 pin | Connects to |
|---|---|
| PB6 (I2C1_SCL) | BMP280 SCL, OLED SCL |
| PB7 (I2C1_SDA) | BMP280 SDA, OLED SDA |
| PA9 (USART1_TX) | USB-TTL adapter RX (to PC) |
| PA10 (USART1_RX) | USB-TTL adapter TX (optional) |
| PA2 (USART2_TX) | ESP32 GPIO16 (RX2) |
| PA3 (USART2_RX) | ESP32 GPIO17 (TX2), optional |
| 3V3 / GND | BMP280, OLED, ESP32 GND |

- BMP280 SDO goes to GND, giving address **0x76**. If yours is 0x77, change `BMP280_I2C_ADDR`.
- OLED address is **0x3C**. If yours is 0x3D, change `OLED_I2C_ADDR`.
- The I2C bus needs pull-ups. Most breakout boards include them; if not, add 4.7 kΩ to 3.3 V on SDA and SCL.
- **Do not** use a 5 V USB-TTL adapter on PA10.

## UART protocol (STM32 to ESP32)

```
$BMP,<temp in 0.01 °C>,<pressure in Pa>*<XOR checksum>\r\n
$BMP,2345,101325*5B        -> 23.45 °C, 1013.25 hPa
```

## Building

**STM32:**
```
cd stm32
make            # build/firmware.elf, .bin, .hex
make flash      # flash via ST-Link (needs st-flash)
```

| File | Purpose |
|---|---|
| `Startup/startup_stm32f4.c` | Vector table for all 86 F401/F411 IRQs (weak handlers), FPU enable, `.data`/`.bss` init, then `main()` |
| `Startup/stm32f4_blackpill.ld` | Linker script. Defaults to **F401CC (256 KB / 64 KB)**, which runs on every Black Pill |
| `Makefile` | `arm-none-eabi-gcc`, hard-float, `-nostdlib` + `-lgcc` |

**Set your chip variant** at the top of `stm32f4_blackpill.ld`, using the marking on the MCU:

| Chip | `FLASH_SIZE` | `RAM_SIZE` |
|---|---|---|
| STM32F401CCU6 | 256K | 64K (default) |
| STM32F401CEU6 | 512K | 96K |
| STM32F411CEU6 | 512K | 128K |

Never set `RAM_SIZE` larger than the real chip has. The stack would start in RAM that doesn't exist,
and the MCU would HardFault immediately.

The clock is left at the 16 MHz HSI reset default (`SYSCLK_HZ` in `stm32f4_reg.h`). If you later
enable the PLL, update `SYSCLK_HZ`, `PCLK1_HZ` and `PCLK2_HZ`.

**Using STM32CubeIDE instead of the Makefile:** create an *Empty* project, then replace its startup
`.s` file with `startup_stm32f4.c` and its `.ld` file with `stm32f4_blackpill.ld`. Use one or the
other; don't keep both startup files.

**ESP32:**
```
cd esp32_ble_gateway
idf.py set-target esp32
idf.py build flash monitor
```


## Viewing on the phone
1. Install **nRF Connect** (Android or iOS).
2. Scan and connect to **BMP280-Room**.
3. Open **Environmental Sensing**, then **Temperature**, and tap the notify icon (the down arrows).
   The value updates every second, for example `23.45 °C`.
=======
### 5. View on the phone
1. Open nRF Connect and connect to **BMP280-Room**.
2. Open **Environmental Sensing**, then **Temperature**.
3. Tap the notify icon. The value updates every second.

## Notes
- The clock is the 16 MHz HSI reset default. If you enable the PLL, update `SYSCLK_HZ`, `PCLK1_HZ` and `PCLK2_HZ` in `stm32f4_reg.h`.
- **STM32CubeIDE users:** create an *Empty* project and replace its startup `.s` and `.ld` files with the ones in `stm32/Startup/`. Don't keep both startup files.

