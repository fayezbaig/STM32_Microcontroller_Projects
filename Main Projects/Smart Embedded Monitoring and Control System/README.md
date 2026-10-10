
# Smart Embedded Monitoring & Control System

An STM32-based embedded systems project developed in Embedded C using bare-metal, register-level programming.

## Overview

The system reads analog input from a potentiometer and sends the measured ADC values to a PC terminal for monitoring. A push button toggles the operating mode, enabling or disabling LED brightness control based on the potentiometer input. PWM is used to adjust LED brightness, while timer interrupts trigger periodic data reporting over UART.

## Key Features

- **ADC:** Reads analog input from a potentiometer connected to PA1.
- **PWM:** Uses TIM2 to control LED brightness based on the ADC reading.
- **UART:** Transmits ADC readings to a PC terminal for monitoring and debugging.
- **External Interrupts:** Uses a push button on PA7 to toggle the system's operating mode.
- **Timer Interrupts:** Uses TIM2 interrupts to trigger periodic monitoring updates.
- **Direct Register Access:** Configures STM32 peripherals through memory-mapped registers without relying on HAL peripheral APIs.

## Technologies

- Embedded C
- STM32 ARM Cortex-M
- Bare-metal register-level programming
- ADC, PWM, UART, GPIO, timers, and external interrupts
- STM32CubeIDE

## Example UART Output

```text
ADC = 2048
ADC = 3010
ADC = 1024
```

## Author

**Fayez Baig**

[GitHub Profile](https://github.com/fayezbaig)
