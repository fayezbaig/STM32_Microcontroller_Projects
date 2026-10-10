/*
 * stm32f4_reg.h
 * Minimal bare-metal register map for STM32F401 / STM32F411 (Black Pill).
 * Only the peripherals used in this project are described:
 *   RCC, GPIOA/B/C, I2C1, USART1, USART2, SysTick.
 * Addresses and bit positions: RM0368 (F401) / RM0383 (F411).
 */
#ifndef STM32F4_REG_H
#define STM32F4_REG_H

#include <stdint.h>

#define __IO volatile

/* ------------------------------------------------------------------ */
/* Clock configuration                                                 */
/* After reset the chip runs from the 16 MHz HSI, AHB/APB1/APB2 = /1.  */
/* This project keeps that default to stay simple and deterministic.   */
/* ------------------------------------------------------------------ */
#define SYSCLK_HZ   16000000UL
#define PCLK1_HZ    16000000UL   /* APB1: I2C1, USART2 */
#define PCLK2_HZ    16000000UL   /* APB2: USART1       */

/* ------------------------------------------------------------------ */
/* RCC                                                                 */
/* ------------------------------------------------------------------ */
typedef struct {
    __IO uint32_t CR;          /* 0x00 */
    __IO uint32_t PLLCFGR;     /* 0x04 */
    __IO uint32_t CFGR;        /* 0x08 */
    __IO uint32_t CIR;         /* 0x0C */
    __IO uint32_t AHB1RSTR;    /* 0x10 */
    __IO uint32_t AHB2RSTR;    /* 0x14 */
    uint32_t      RESERVED0[2];/* 0x18-0x1C */
    __IO uint32_t APB1RSTR;    /* 0x20 */
    __IO uint32_t APB2RSTR;    /* 0x24 */
    uint32_t      RESERVED1[2];/* 0x28-0x2C */
    __IO uint32_t AHB1ENR;     /* 0x30 */
    __IO uint32_t AHB2ENR;     /* 0x34 */
    uint32_t      RESERVED2[2];/* 0x38-0x3C */
    __IO uint32_t APB1ENR;     /* 0x40 */
    __IO uint32_t APB2ENR;     /* 0x44 */
} RCC_TypeDef;

#define RCC ((RCC_TypeDef *)0x40023800UL)

#define RCC_AHB1ENR_GPIOAEN   (1UL << 0)
#define RCC_AHB1ENR_GPIOBEN   (1UL << 1)
#define RCC_AHB1ENR_GPIOCEN   (1UL << 2)
#define RCC_APB1ENR_USART2EN  (1UL << 17)
#define RCC_APB1ENR_I2C1EN    (1UL << 21)
#define RCC_APB1RSTR_I2C1RST  (1UL << 21)
#define RCC_APB2ENR_USART1EN  (1UL << 4)

/* ------------------------------------------------------------------ */
/* GPIO                                                                */
/* ------------------------------------------------------------------ */
typedef struct {
    __IO uint32_t MODER;    /* 0x00 */
    __IO uint32_t OTYPER;   /* 0x04 */
    __IO uint32_t OSPEEDR;  /* 0x08 */
    __IO uint32_t PUPDR;    /* 0x0C */
    __IO uint32_t IDR;      /* 0x10 */
    __IO uint32_t ODR;      /* 0x14 */
    __IO uint32_t BSRR;     /* 0x18 */
    __IO uint32_t LCKR;     /* 0x1C */
    __IO uint32_t AFR[2];   /* 0x20-0x24 */
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef *)0x40020000UL)
#define GPIOB ((GPIO_TypeDef *)0x40020400UL)
#define GPIOC ((GPIO_TypeDef *)0x40020800UL)

/* ------------------------------------------------------------------ */
/* I2C (classic F1/F2/F4 style peripheral)                             */
/* ------------------------------------------------------------------ */
typedef struct {
    __IO uint32_t CR1;    /* 0x00 */
    __IO uint32_t CR2;    /* 0x04 */
    __IO uint32_t OAR1;   /* 0x08 */
    __IO uint32_t OAR2;   /* 0x0C */
    __IO uint32_t DR;     /* 0x10 */
    __IO uint32_t SR1;    /* 0x14 */
    __IO uint32_t SR2;    /* 0x18 */
    __IO uint32_t CCR;    /* 0x1C */
    __IO uint32_t TRISE;  /* 0x20 */
    __IO uint32_t FLTR;   /* 0x24 */
} I2C_TypeDef;

#define I2C1 ((I2C_TypeDef *)0x40005400UL)

#define I2C_CR1_PE      (1UL << 0)
#define I2C_CR1_START   (1UL << 8)
#define I2C_CR1_STOP    (1UL << 9)
#define I2C_CR1_ACK     (1UL << 10)
#define I2C_CR1_SWRST   (1UL << 15)

#define I2C_SR1_SB      (1UL << 0)
#define I2C_SR1_ADDR    (1UL << 1)
#define I2C_SR1_BTF     (1UL << 2)
#define I2C_SR1_RXNE    (1UL << 6)
#define I2C_SR1_TXE     (1UL << 7)
#define I2C_SR1_BERR    (1UL << 8)
#define I2C_SR1_ARLO    (1UL << 9)
#define I2C_SR1_AF      (1UL << 10)

#define I2C_SR2_BUSY    (1UL << 1)

#define I2C_CCR_FS      (1UL << 15)

/* ------------------------------------------------------------------ */
/* USART                                                               */
/* ------------------------------------------------------------------ */
typedef struct {
    __IO uint32_t SR;    /* 0x00 */
    __IO uint32_t DR;    /* 0x04 */
    __IO uint32_t BRR;   /* 0x08 */
    __IO uint32_t CR1;   /* 0x0C */
    __IO uint32_t CR2;   /* 0x10 */
    __IO uint32_t CR3;   /* 0x14 */
    __IO uint32_t GTPR;  /* 0x18 */
} USART_TypeDef;

#define USART1 ((USART_TypeDef *)0x40011000UL)
#define USART2 ((USART_TypeDef *)0x40004400UL)

#define USART_SR_RXNE   (1UL << 5)
#define USART_SR_TC     (1UL << 6)
#define USART_SR_TXE    (1UL << 7)

#define USART_CR1_RE    (1UL << 2)
#define USART_CR1_TE    (1UL << 3)
#define USART_CR1_UE    (1UL << 13)

/* ------------------------------------------------------------------ */
/* SysTick (Cortex-M4 core peripheral)                                 */
/* ------------------------------------------------------------------ */
typedef struct {
    __IO uint32_t CTRL;
    __IO uint32_t LOAD;
    __IO uint32_t VAL;
    __IO uint32_t CALIB;
} SysTick_TypeDef;

#define SYSTICK ((SysTick_TypeDef *)0xE000E010UL)

#define SYSTICK_CTRL_ENABLE    (1UL << 0)
#define SYSTICK_CTRL_TICKINT   (1UL << 1)
#define SYSTICK_CTRL_CLKSOURCE (1UL << 2)   /* 1 = processor clock */

#endif /* STM32F4_REG_H */
