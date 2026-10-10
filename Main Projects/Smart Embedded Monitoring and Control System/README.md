# Potentiometer-Controlled LED Dimmer with Interrupt-Driven Mode Switching

*STM32F401/F411 Black Pill, bare metal (register level, no HAL)*

Turn a potentiometer and an LED gets brighter or darker. Press a button to switch the dimmer on or off.
The board reports the knob position and the brightness to a PC terminal five times a second.

The system reads analog input from a potentiometer and sends the measured ADC values to a PC terminal for monitoring. A push button toggles the operating mode, enabling or disabling LED brightness control based on the potentiometer input. PWM is used to adjust LED brightness, while timer interrupts trigger periodic data reporting over UART.

## Key Features

- **ADC:** Reads analog input from a potentiometer connected to PA1.
- **PWM:** Uses TIM2 Channel 1 on PA0 to control LED brightness based on the ADC reading.
- **UART:** Transmits ADC readings to a PC terminal for monitoring and debugging.
- **External Interrupts:** Uses a push button on PA7 to toggle the system's operating mode.
- **Timer Interrupts:** Uses TIM2 interrupts to trigger periodic monitoring updates.
- **Bare-Metal Programming:** Configures STM32 peripherals through memory-mapped registers without relying on HAL peripheral APIs.

## Files

| File | Purpose |
|---|---|
| `Inc/stm32f4_reg.h` | Register map: RCC, GPIO, USART, ADC, TIM, SYSCFG, EXTI, NVIC, SysTick |
| `Inc/gpio.h`, `Src/gpio.c` | Pin configuration, on-board LED (PC13) |
| `Inc/systick.h`, `Src/systick.c` | 1 ms tick (`millis()`), used for debouncing |
| `Inc/uart.h`, `Src/uart.c` | USART2 output, 9600 8N1 |
| `Inc/adc.h`, `Src/adc.c` | ADC1 channel 1 (PA1), 12-bit |
| `Inc/pwm.h`, `Src/pwm.c` | TIM2 CH1 PWM on PA0, 1 kHz |
| `Inc/timer.h`, `Src/timer.c` | TIM3 periodic interrupt that triggers the UART report |
| `Inc/button.h`, `Src/button.c` | PA7 button, EXTI interrupt, 50 ms debounce |
| `Src/main.c` | Application: mode switching, pot to brightness, status output |
| `Startup/startup_stm32f4.c` | Vector table, FPU enable, `.data`/`.bss` init, then calls `main()` |
| `Startup/stm32f4_blackpill.ld` | Linker script (chip flash/RAM sizes) |
| `Makefile` | `make` builds, `make flash` flashes via ST-Link |

## Wiring

| STM32 pin | Connects to |
|---|---|
| PA0 (TIM2_CH1) | 330 Ω resistor → LED anode, LED cathode → GND |
| PA1 (ADC1_IN1) | Potentiometer wiper. Outer pins go to 3V3 and GND |
| PA7 | Push button → GND (internal pull-up is used) |
| PA2 (USART2_TX) | USB-TTL adapter RX |
| GND | USB-TTL adapter GND |

> **Note:** on the Black Pill, PA0 is also wired to the on-board **KEY** button. Don't press KEY while
> the dimmer is running, because it shorts the PWM output to GND.

## How to run

**You need:** `arm-none-eabi-gcc` + `make` (or STM32CubeIDE), an ST-Link or `dfu-util`, and a USB-TTL adapter.

### 1. Set your chip
Open `Startup/stm32f4_blackpill.ld` and set `FLASH_SIZE` and `RAM_SIZE` to match the marking on the MCU:

| Chip | `FLASH_SIZE` | `RAM_SIZE` |
|---|---|---|
| STM32F401CCU6 | 256K | 64K (default, works on all) |
| STM32F401CEU6 | 512K | 96K |
| STM32F411CEU6 | 512K | 128K |

### 2. Build and flash
```
make
make flash
```
`make flash` uses an ST-Link. Without one, you can flash over USB-C instead:
1. Hold **BOOT0**, tap **NRST**, then release BOOT0.
2. Run:
```
dfu-util -a 0 -s 0x08000000:leave -D build/firmware.bin
```

### 3. Use it
Open the USB-TTL adapter's COM port at **9600 baud** and press NRST.
1. Press the button on PA7. The on-board LED lights up and the mode changes to DIMMER.
2. Turn the potentiometer. The LED on PA0 dims smoothly, and the terminal shows:
```
Mode: DIMMER | ADC: 2048 | Duty:  50 %
```
3. Press the button again to switch the LED off.

## Example UART Output

```text
ADC = 2048
ADC = 3010
ADC = 1024
