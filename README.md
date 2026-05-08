# Embedded Systems Projects (STM32 / ARM Cortex-M)

This repository contains a collection of embedded systems projects developed using STM32 microcontrollers based on the ARM Cortex-M architecture. The projects focus on low-level peripheral programming using bare-metal register-level development and fundamental embedded systems concepts.

---

# 📌 Overview

The purpose of this repository is to demonstrate practical understanding of:

- Embedded C programming
- STM32 peripheral configuration
- Register-level programming
- Real-time embedded system behavior
- Hardware-software interaction
- Interrupt-driven systems

The projects range from beginner to intermediate level and cover multiple STM32 peripherals including GPIO, UART, I2C, ADC, PWM, Timers, and External Interrupts.

---

# 🔧 Topics Covered

---

# 1. GPIO (General Purpose Input/Output)

Projects related to digital input and output operations.

## Features
- LED ON/OFF control
- Push button interfacing
- Input pull-up and pull-down configuration
- GPIO register programming
- Hardware interaction using STM32 pins

## Example Projects
- LED Blink
- Button Controlled LED
- Multiple LED Control
- GPIO-based state machines

---

# 2. External Interrupts (EXTI)

Projects demonstrating interrupt-driven programming.

## Features
- Configuring EXTI lines
- NVIC interrupt handling
- Falling/Rising edge detection
- Real-time button event handling
- Interrupt priority configuration

## Example Projects
- Button Interrupt LED Toggle
- Multi-button Interrupt System
- Interrupt-based mode switching
- Event-driven embedded applications

---

# 3. UART Communication

Projects demonstrating serial communication using UART.

## Features
- UART Transmit and Receive
- Polling and interrupt-based communication
- Serial debugging
- Data transmission to PC terminal
- Command-based communication systems

## Example Projects
- UART Transmitter
- UART Receiver
- Echo System
- Interrupt-based UART communication

---

# 4. I2C Communication

Projects demonstrating inter-device communication using I2C protocol.

## Features
- I2C Master Transmit/Receive
- EEPROM communication
- Polling and interrupt-based I2C
- Register-level I2C configuration
- Sensor communication basics

## Example Projects
- I2C Master Transmitter
- I2C Master Receiver
- EEPROM Write System
- Interrupt-based I2C Receiver

---

# 5. PWM (Pulse Width Modulation)

Projects focused on PWM signal generation using timers.

## Features
- PWM signal generation
- LED brightness control
- Servo motor control
- Duty cycle variation
- Timer PWM modes

## Example Projects
- PWM LED Brightness
- LED Fade Effect
- Servo Motor Control
- PWM Interrupt Applications

---

# 6. ADC (Analog to Digital Converter)

Projects related to analog signal acquisition and processing.

## Features
- Analog voltage reading
- Potentiometer interfacing
- Sensor data acquisition
- Interrupt-based ADC conversion
- ADC + PWM integration

## Example Projects
- Potentiometer Reader
- ADC Controlled PWM
- LM35 Temperature Sensor
- Interrupt-based ADC System

---

# 7. Timers

Projects demonstrating STM32 timer peripherals.

## Features
- Timer configuration
- Hardware delays
- Periodic interrupts
- Input Capture
- Output Compare
- Frequency measurement

## Example Projects
- Timer Delay Generation
- Timer Interrupt Blinker
- Input Capture Frequency Meter
- Output Compare Toggle Mode

---

# 8. Integrated Embedded Systems Projects

Intermediate-level projects combining multiple peripherals into a complete embedded application.

## Features
- ADC + PWM integration
- UART monitoring
- I2C communication
- GPIO interrupt handling
- Timer scheduling
- Multi-peripheral coordination

## Example Projects
- Smart Embedded Monitoring System
- Sensor Monitoring & PWM Control
- Interrupt-driven embedded applications

---

# 🛠 Tools & Technologies

- STM32 Microcontrollers (ARM Cortex-M series)
- STM32CubeIDE
- Embedded C Programming
- Bare-Metal Register-Level Programming
- STM32 HAL (where applicable)
- Git & GitHub for version control

---

# 📚 Programming Approach

Most projects are implemented using:

- Direct register manipulation
- Peripheral register configuration
- Bare-metal embedded programming
- Minimal abstraction layers

The purpose is to build strong understanding of how STM32 peripherals operate internally.

---

# 📁 Repository Structure

Each folder contains a standalone STM32 project.

```text
GPIO_Project/
Interrupt_Project/
UART_Projects/
I2C_Projects/
PWM_Projects/
ADC_Projects/
Timer_Projects/
Integrated_Projects/
