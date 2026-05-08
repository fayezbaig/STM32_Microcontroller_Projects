# Smart Embedded Monitoring & Control System

An intermediate-level embedded systems project developed using STM32 microcontrollers and bare-metal register-level programming.

This project integrates multiple STM32 peripherals into a single real-time embedded application including:

- ADC
- PWM
- UART
- I2C
- GPIO
- External Interrupts
- Timers

---

# 📌 Project Overview

The purpose of this project is to demonstrate how multiple peripherals inside an STM32 microcontroller can work together in a complete embedded system.

The system reads analog sensor data using ADC, controls LED brightness using PWM, communicates data through UART and I2C, and uses interrupts for responsive real-time behavior.

This project is implemented entirely using:

- Embedded C
- Bare-metal register programming
- STM32 ARM Cortex-M microcontroller

---

# ⚙ Features

## ✅ ADC Sensor Reading
- Reads analog voltage from a potentiometer connected to PA1
- Continuous ADC conversion
- Real-time analog monitoring

---

## ✅ PWM LED Brightness Control
- PWM signal generated using TIM2
- ADC value controls LED brightness
- Real-time duty cycle adjustment

---

## ✅ UART Communication
- Sends ADC values to Serial Monitor / PC terminal
- Useful for debugging and monitoring

Example UART Output:

```text
ADC = 2048
ADC = 3010
ADC = 1024
