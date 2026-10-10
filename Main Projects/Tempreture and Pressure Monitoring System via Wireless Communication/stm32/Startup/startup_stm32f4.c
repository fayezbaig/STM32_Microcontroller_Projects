/*
 * startup_stm32f4.c - bare-metal startup code for STM32F401 / STM32F411
 *
 * Written in C (no assembly needed on Cortex-M): the core loads the initial
 * stack pointer from vector[0] and jumps to vector[1] (Reset_Handler) by
 * itself, so C code can run from the very first instruction.
 *
 * Reset_Handler:
 *   1. enable the FPU (harmless if you compile with soft-float)
 *   2. copy .data from flash to RAM
 *   3. zero .bss
 *   4. run static constructors (.preinit_array / .init_array)
 *   5. call main()
 *
 * Every interrupt handler is a weak alias of Default_Handler. Defining a
 * function with the same name anywhere (e.g. SysTick_Handler in systick.c)
 * overrides it automatically.
 *
 * Vector table: RM0368 (F401) table 38 / RM0383 (F411) table 37.
 */
#include <stdint.h>

/* ------------------------------------------------------------------ */
/* Symbols from the linker script                                      */
/* ------------------------------------------------------------------ */
extern uint32_t _estack;    /* top of RAM = initial stack pointer */
extern uint32_t _sidata;    /* .data load address in flash        */
extern uint32_t _sdata;     /* .data start in RAM                 */
extern uint32_t _edata;     /* .data end in RAM                   */
extern uint32_t _sbss;      /* .bss start                         */
extern uint32_t _ebss;      /* .bss end                           */

extern void (*__preinit_array_start[])(void);
extern void (*__preinit_array_end[])(void);
extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);

int main(void);

/* Coprocessor Access Control Register (System Control Block) */
#define SCB_CPACR (*(volatile uint32_t *)0xE000ED88UL)

/* ------------------------------------------------------------------ */
/* Default handlers                                                    */
/* ------------------------------------------------------------------ */

/* Any interrupt without its own handler ends up here. If the program
 * hangs here, look at the IPSR register in the debugger: it holds the
 * number of the exception that fired. */
void Default_Handler(void)
{
    for (;;) { }
}

/* Separate HardFault loop so it is easy to recognise in the debugger */
__attribute__((weak)) void HardFault_Handler(void)
{
    for (;;) { }
}

#define WEAK_ALIAS __attribute__((weak, alias("Default_Handler")))

/* Cortex-M4 core exceptions */
void NMI_Handler(void)          WEAK_ALIAS;
void MemManage_Handler(void)    WEAK_ALIAS;
void BusFault_Handler(void)     WEAK_ALIAS;
void UsageFault_Handler(void)   WEAK_ALIAS;
void SVC_Handler(void)          WEAK_ALIAS;
void DebugMon_Handler(void)     WEAK_ALIAS;
void PendSV_Handler(void)       WEAK_ALIAS;
void SysTick_Handler(void)      WEAK_ALIAS;

/* STM32F401/F411 peripheral interrupts */
void WWDG_IRQHandler(void)               WEAK_ALIAS;
void PVD_IRQHandler(void)                WEAK_ALIAS;
void TAMP_STAMP_IRQHandler(void)         WEAK_ALIAS;
void RTC_WKUP_IRQHandler(void)           WEAK_ALIAS;
void FLASH_IRQHandler(void)              WEAK_ALIAS;
void RCC_IRQHandler(void)                WEAK_ALIAS;
void EXTI0_IRQHandler(void)              WEAK_ALIAS;
void EXTI1_IRQHandler(void)              WEAK_ALIAS;
void EXTI2_IRQHandler(void)              WEAK_ALIAS;
void EXTI3_IRQHandler(void)              WEAK_ALIAS;
void EXTI4_IRQHandler(void)              WEAK_ALIAS;
void DMA1_Stream0_IRQHandler(void)       WEAK_ALIAS;
void DMA1_Stream1_IRQHandler(void)       WEAK_ALIAS;
void DMA1_Stream2_IRQHandler(void)       WEAK_ALIAS;
void DMA1_Stream3_IRQHandler(void)       WEAK_ALIAS;
void DMA1_Stream4_IRQHandler(void)       WEAK_ALIAS;
void DMA1_Stream5_IRQHandler(void)       WEAK_ALIAS;
void DMA1_Stream6_IRQHandler(void)       WEAK_ALIAS;
void ADC_IRQHandler(void)                WEAK_ALIAS;
void EXTI9_5_IRQHandler(void)            WEAK_ALIAS;
void TIM1_BRK_TIM9_IRQHandler(void)      WEAK_ALIAS;
void TIM1_UP_TIM10_IRQHandler(void)      WEAK_ALIAS;
void TIM1_TRG_COM_TIM11_IRQHandler(void) WEAK_ALIAS;
void TIM1_CC_IRQHandler(void)            WEAK_ALIAS;
void TIM2_IRQHandler(void)               WEAK_ALIAS;
void TIM3_IRQHandler(void)               WEAK_ALIAS;
void TIM4_IRQHandler(void)               WEAK_ALIAS;
void I2C1_EV_IRQHandler(void)            WEAK_ALIAS;
void I2C1_ER_IRQHandler(void)            WEAK_ALIAS;
void I2C2_EV_IRQHandler(void)            WEAK_ALIAS;
void I2C2_ER_IRQHandler(void)            WEAK_ALIAS;
void SPI1_IRQHandler(void)               WEAK_ALIAS;
void SPI2_IRQHandler(void)               WEAK_ALIAS;
void USART1_IRQHandler(void)             WEAK_ALIAS;
void USART2_IRQHandler(void)             WEAK_ALIAS;
void EXTI15_10_IRQHandler(void)          WEAK_ALIAS;
void RTC_Alarm_IRQHandler(void)          WEAK_ALIAS;
void OTG_FS_WKUP_IRQHandler(void)        WEAK_ALIAS;
void DMA1_Stream7_IRQHandler(void)       WEAK_ALIAS;
void SDIO_IRQHandler(void)               WEAK_ALIAS;
void TIM5_IRQHandler(void)               WEAK_ALIAS;
void SPI3_IRQHandler(void)               WEAK_ALIAS;
void DMA2_Stream0_IRQHandler(void)       WEAK_ALIAS;
void DMA2_Stream1_IRQHandler(void)       WEAK_ALIAS;
void DMA2_Stream2_IRQHandler(void)       WEAK_ALIAS;
void DMA2_Stream3_IRQHandler(void)       WEAK_ALIAS;
void DMA2_Stream4_IRQHandler(void)       WEAK_ALIAS;
void OTG_FS_IRQHandler(void)             WEAK_ALIAS;
void DMA2_Stream5_IRQHandler(void)       WEAK_ALIAS;
void DMA2_Stream6_IRQHandler(void)       WEAK_ALIAS;
void DMA2_Stream7_IRQHandler(void)       WEAK_ALIAS;
void USART6_IRQHandler(void)             WEAK_ALIAS;
void I2C3_EV_IRQHandler(void)            WEAK_ALIAS;
void I2C3_ER_IRQHandler(void)            WEAK_ALIAS;
void FPU_IRQHandler(void)                WEAK_ALIAS;
void SPI4_IRQHandler(void)               WEAK_ALIAS;
void SPI5_IRQHandler(void)               WEAK_ALIAS;   /* F411 only */

/* ------------------------------------------------------------------ */
/* Reset handler                                                       */
/* ------------------------------------------------------------------ */
void Reset_Handler(void)
{
    /* 1. FPU: full access for CP10 and CP11. Must happen before any
     *    floating-point instruction, i.e. before .data copy in case the
     *    compiler uses FP registers for memcpy-like code. */
    SCB_CPACR |= (0xFUL << 20);
    __asm volatile ("dsb\n\tisb" ::: "memory");

    /* 2. Copy initialised data from flash to RAM */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* 3. Zero-fill uninitialised data */
    for (dst = &_sbss; dst < &_ebss; ) {
        *dst++ = 0;
    }

    /* 4. Static constructors (only used by C++ or __attribute__((constructor))) */
    for (void (**f)(void) = __preinit_array_start; f < __preinit_array_end; f++) (*f)();
    for (void (**f)(void) = __init_array_start;    f < __init_array_end;    f++) (*f)();

    /* 5. Application */
    (void)main();

    /* main() should never return; if it does, stay here */
    for (;;) { }
}

/* ------------------------------------------------------------------ */
/* Vector table - placed at 0x08000000 by the linker script            */
/* ------------------------------------------------------------------ */
typedef void (*vector_t)(void);

__attribute__((section(".isr_vector"), used))
const vector_t g_vector_table[16 + 86] = {
    /* ---- Cortex-M4 core ---- */
    (vector_t)&_estack,             /* 0  initial stack pointer */
    Reset_Handler,                  /* 1  */
    NMI_Handler,                    /* 2  */
    HardFault_Handler,              /* 3  */
    MemManage_Handler,              /* 4  */
    BusFault_Handler,               /* 5  */
    UsageFault_Handler,             /* 6  */
    0, 0, 0, 0,                     /* 7-10 reserved */
    SVC_Handler,                    /* 11 */
    DebugMon_Handler,               /* 12 */
    0,                              /* 13 reserved */
    PendSV_Handler,                 /* 14 */
    SysTick_Handler,                /* 15 */

    /* ---- STM32F401/F411 IRQ0..IRQ85 ---- */
    WWDG_IRQHandler,                /* 0  */
    PVD_IRQHandler,                 /* 1  */
    TAMP_STAMP_IRQHandler,          /* 2  */
    RTC_WKUP_IRQHandler,            /* 3  */
    FLASH_IRQHandler,               /* 4  */
    RCC_IRQHandler,                 /* 5  */
    EXTI0_IRQHandler,               /* 6  */
    EXTI1_IRQHandler,               /* 7  */
    EXTI2_IRQHandler,               /* 8  */
    EXTI3_IRQHandler,               /* 9  */
    EXTI4_IRQHandler,               /* 10 */
    DMA1_Stream0_IRQHandler,        /* 11 */
    DMA1_Stream1_IRQHandler,        /* 12 */
    DMA1_Stream2_IRQHandler,        /* 13 */
    DMA1_Stream3_IRQHandler,        /* 14 */
    DMA1_Stream4_IRQHandler,        /* 15 */
    DMA1_Stream5_IRQHandler,        /* 16 */
    DMA1_Stream6_IRQHandler,        /* 17 */
    ADC_IRQHandler,                 /* 18 */
    0, 0, 0, 0,                     /* 19-22 reserved */
    EXTI9_5_IRQHandler,             /* 23 */
    TIM1_BRK_TIM9_IRQHandler,       /* 24 */
    TIM1_UP_TIM10_IRQHandler,       /* 25 */
    TIM1_TRG_COM_TIM11_IRQHandler,  /* 26 */
    TIM1_CC_IRQHandler,             /* 27 */
    TIM2_IRQHandler,                /* 28 */
    TIM3_IRQHandler,                /* 29 */
    TIM4_IRQHandler,                /* 30 */
    I2C1_EV_IRQHandler,             /* 31 */
    I2C1_ER_IRQHandler,             /* 32 */
    I2C2_EV_IRQHandler,             /* 33 */
    I2C2_ER_IRQHandler,             /* 34 */
    SPI1_IRQHandler,                /* 35 */
    SPI2_IRQHandler,                /* 36 */
    USART1_IRQHandler,              /* 37 */
    USART2_IRQHandler,              /* 38 */
    0,                              /* 39 reserved */
    EXTI15_10_IRQHandler,           /* 40 */
    RTC_Alarm_IRQHandler,           /* 41 */
    OTG_FS_WKUP_IRQHandler,         /* 42 */
    0, 0, 0, 0,                     /* 43-46 reserved */
    DMA1_Stream7_IRQHandler,        /* 47 */
    0,                              /* 48 reserved */
    SDIO_IRQHandler,                /* 49 */
    TIM5_IRQHandler,                /* 50 */
    SPI3_IRQHandler,                /* 51 */
    0, 0, 0, 0,                     /* 52-55 reserved */
    DMA2_Stream0_IRQHandler,        /* 56 */
    DMA2_Stream1_IRQHandler,        /* 57 */
    DMA2_Stream2_IRQHandler,        /* 58 */
    DMA2_Stream3_IRQHandler,        /* 59 */
    DMA2_Stream4_IRQHandler,        /* 60 */
    0, 0, 0, 0, 0, 0,               /* 61-66 reserved */
    OTG_FS_IRQHandler,              /* 67 */
    DMA2_Stream5_IRQHandler,        /* 68 */
    DMA2_Stream6_IRQHandler,        /* 69 */
    DMA2_Stream7_IRQHandler,        /* 70 */
    USART6_IRQHandler,              /* 71 */
    I2C3_EV_IRQHandler,             /* 72 */
    I2C3_ER_IRQHandler,             /* 73 */
    0, 0, 0, 0, 0, 0, 0,            /* 74-80 reserved */
    FPU_IRQHandler,                 /* 81 */
    0, 0,                           /* 82-83 reserved */
    SPI4_IRQHandler,                /* 84 */
    SPI5_IRQHandler,                /* 85 (F411 only) */
};
