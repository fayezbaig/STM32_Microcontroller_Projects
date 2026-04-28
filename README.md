# Embedded Systems Projects (STM32 / ARM Cortex-M)

This repository contains a collection of embedded systems projects developed using STM32 microcontrollers based on the ARM Cortex-M architecture. The projects focus on fundamental embedded concepts such as GPIO control, external interrupts, and timer-based applications.

## 📌 Overview

The goal of this repository is to demonstrate practical understanding of core microcontroller peripherals and low-level embedded programming. Each project is implemented using C and STM32 HAL / register-level programming depending on the use case.

## 🔧 Topics Covered

### 1. GPIO (General Purpose Input/Output)
- Digital input/output configuration
- LED control and switching logic
- Button interfacing with debouncing
- Basic hardware interaction using STM32 pins

### 2. External Interrupts (EXTI)
- Configuring interrupt lines for GPIO pins
- Event-driven programming using hardware interrupts
- Handling real-time button presses and external signals
- Interrupt priority configuration

### 3. Timers
- Basic timer configuration (polling and interrupt mode)
- Generating delays using hardware timers
- Periodic task execution using timer interrupts
- PWM signal generation (where applicable)

## 🛠 Tools & Technologies

- STM32 Microcontrollers (ARM Cortex-M series)
- STM32CubeIDE
- C Programming
- STM32 HAL / Register-level programming
- Git for version control

## 🎯 Purpose of Projects

These implementations were developed to strengthen understanding of:
- Low-level embedded system design
- Microcontroller peripheral configuration
- Real-time system behavior
- Hardware-software interaction

## 📁 Repository Structure

Each folder contains an individual project focusing on a specific concept:

- `GPIO_Project/` – Basic digital input/output operations
- `Interrupt_Project/` – External interrupt handling examples
- `Timer_Project/` – Timer-based applications and delays

## 🚀 Future Improvements

- Integration with sensors (I2C / SPI)
- UART-based communication projects
- RTOS-based task scheduling examples
- More advanced embedded system applications

## 📌 Note

This repository is intended for educational and learning purposes in embedded systems development.
