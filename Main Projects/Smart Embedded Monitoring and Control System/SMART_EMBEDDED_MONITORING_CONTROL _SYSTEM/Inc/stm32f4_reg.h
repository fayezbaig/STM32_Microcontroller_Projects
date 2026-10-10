/*
 * stm32f4_reg.h
 * Minimal bare-metal register map for STM32F401 / STM32F411 (Black Pill).
 * Only the peripherals used in this project are described:
 *   RCC, GPIOA/B/C, USART2, ADC1, TIM2, TIM3, SYSCFG, EXTI, NVIC, SysTick.
 * Addresses and bit positions: RM0368 (F401) / RM0383 (F411).
 *
 * Every register is accessed through a 'volatile' struct member, so the
 * compiler re-reads the hardware on every access (safe at any -O level).
 */
#ifndef STM32F4_REG_H
#define STM32F4_REG_H

#include <stdint.h>

#define __IO volatile

/* ------------------------------------------------------------------ */
/* Clock: 16 MHz HSI reset default, all bus prescalers /1              */
/* ------------------------------------------------------------------ */
#define SYSCLK_HZ   16000000UL
#define PCLK1_HZ    16000000UL   /* APB1: USART2, TIM2, TIM3 */
#define PCLK2_HZ    16000000UL   /* APB2: USART1, ADC1        */
#define TIMCLK1_HZ  PCLK1_HZ     /* APB1 prescaler = 1 -> timer clock = PCLK1 */

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
#define RCC_APB1ENR_TIM2EN    (1UL << 0)
#define RCC_APB1ENR_TIM3EN    (1UL << 1)
#define RCC_APB1ENR_USART2EN  (1UL << 17)
#define RCC_APB2ENR_USART1EN  (1UL << 4)
#define RCC_APB2ENR_ADC1EN    (1UL << 8)
#define RCC_APB2ENR_SYSCFGEN  (1UL << 14)

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
/* ADC1                                                                */
/* ------------------------------------------------------------------ */
typedef struct {
    __IO uint32_t SR;      /* 0x00 */
    __IO uint32_t CR1;     /* 0x04 */
    __IO uint32_t CR2;     /* 0x08 */
    __IO uint32_t SMPR1;   /* 0x0C  channels 10-18 */
    __IO uint32_t SMPR2;   /* 0x10  channels 0-9   */
    __IO uint32_t JOFR[4]; /* 0x14-0x20 */
    __IO uint32_t HTR;     /* 0x24 */
    __IO uint32_t LTR;     /* 0x28 */
    __IO uint32_t SQR1;    /* 0x2C */
    __IO uint32_t SQR2;    /* 0x30 */
    __IO uint32_t SQR3;    /* 0x34 */
    __IO uint32_t JSQR;    /* 0x38 */
    __IO uint32_t JDR[4];  /* 0x3C-0x48 */
    __IO uint32_t DR;      /* 0x4C */
} ADC_TypeDef;

#define ADC1 ((ADC_TypeDef *)0x40012000UL)

#define ADC_SR_EOC       (1UL << 1)
#define ADC_CR2_ADON     (1UL << 0)
#define ADC_CR2_SWSTART  (1UL << 30)

/* ------------------------------------------------------------------ */
/* General-purpose timers (TIM2..TIM5)                                 */
/* ------------------------------------------------------------------ */
typedef struct {
    __IO uint32_t CR1;    /* 0x00 */
    __IO uint32_t CR2;    /* 0x04 */
    __IO uint32_t SMCR;   /* 0x08 */
    __IO uint32_t DIER;   /* 0x0C */
    __IO uint32_t SR;     /* 0x10 */
    __IO uint32_t EGR;    /* 0x14 */
    __IO uint32_t CCMR1;  /* 0x18 */
    __IO uint32_t CCMR2;  /* 0x1C */
    __IO uint32_t CCER;   /* 0x20 */
    __IO uint32_t CNT;    /* 0x24 */
    __IO uint32_t PSC;    /* 0x28 */
    __IO uint32_t ARR;    /* 0x2C */
    uint32_t      RESERVED0; /* 0x30 */
    __IO uint32_t CCR1;   /* 0x34 */
    __IO uint32_t CCR2;   /* 0x38 */
    __IO uint32_t CCR3;   /* 0x3C */
    __IO uint32_t CCR4;   /* 0x40 */
} TIM_TypeDef;

#define TIM2 ((TIM_TypeDef *)0x40000000UL)
#define TIM3 ((TIM_TypeDef *)0x40000400UL)

#define TIM_CR1_CEN       (1UL << 0)
#define TIM_CR1_ARPE      (1UL << 7)
#define TIM_DIER_UIE      (1UL << 0)
#define TIM_SR_UIF        (1UL << 0)
#define TIM_EGR_UG        (1UL << 0)
#define TIM_CCMR1_OC1PE   (1UL << 3)
#define TIM_CCMR1_OC1M_Pos 4U
#define TIM_OCMODE_PWM1   6UL
#define TIM_CCER_CC1E     (1UL << 0)

/* ------------------------------------------------------------------ */
/* SYSCFG + EXTI                                                       */
/* ------------------------------------------------------------------ */
typedef struct {
    __IO uint32_t MEMRMP;     /* 0x00 */
    __IO uint32_t PMC;        /* 0x04 */
    __IO uint32_t EXTICR[4];  /* 0x08-0x14  EXTICR1..4 */
} SYSCFG_TypeDef;

#define SYSCFG ((SYSCFG_TypeDef *)0x40013800UL)

typedef struct {
    __IO uint32_t IMR;    /* 0x00 */
    __IO uint32_t EMR;    /* 0x04 */
    __IO uint32_t RTSR;   /* 0x08 */
    __IO uint32_t FTSR;   /* 0x0C */
    __IO uint32_t SWIER;  /* 0x10 */
    __IO uint32_t PR;     /* 0x14 */
} EXTI_TypeDef;

#define EXTI ((EXTI_TypeDef *)0x40013C00UL)

/* ------------------------------------------------------------------ */
/* NVIC                                                                */
/* ------------------------------------------------------------------ */
#define NVIC_ISER ((__IO uint32_t *)0xE000E100UL)   /* ISER0..ISER7 */

#define EXTI9_5_IRQn   23U
#define TIM2_IRQn      28U
#define TIM3_IRQn      29U

/* ISER is write-1-to-set: a plain write enables only that IRQ */
static inline void nvic_enable_irq(uint32_t irqn)
{
    NVIC_ISER[irqn >> 5] = 1UL << (irqn & 31U);
}

/* ------------------------------------------------------------------ */
/* SysTick                                                             */
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
#define SYSTICK_CTRL_CLKSOURCE (1UL << 2)

#endif /* STM32F4_REG_H */
