/*
 * stm32f4_blackpill.ld - linker script for STM32F401 / STM32F411 Black Pill
 *
 * DEFAULT = STM32F401CC (256 KB flash, 64 KB RAM). This is the smallest
 * Black Pill variant, so the image runs on ALL of them. If you have a
 * bigger chip, change FLASH_SIZE / RAM_SIZE below:
 *
 *   Chip (marking on the MCU)   FLASH_SIZE   RAM_SIZE
 *   STM32F401CCU6               256K         64K     <- default
 *   STM32F401CEU6               512K         96K
 *   STM32F411CEU6               512K         128K
 *
 * Getting RAM_SIZE too BIG is fatal: the stack would start in
 * non-existent RAM and the MCU HardFaults on the first push.
 */

FLASH_SIZE = 256K;
RAM_SIZE   = 64K;

/* Minimum free space required, checked at link time */
_Min_Stack_Size = 0x800;   /* 2 KB */
_Min_Heap_Size  = 0x0;     /* no malloc in this project */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = FLASH_SIZE
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = RAM_SIZE
}

/* Initial stack pointer: top of the RAM that really exists on this chip */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* Vector table must be the first thing in flash (0x08000000) */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* Code and constants */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7)          /* ARM/Thumb interworking glue */
        *(.glue_7t)
        *(.eh_frame)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* Exception unwinding tables (needed if libgcc pulls them in) */
    .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH

    /* Constructor tables used by Reset_Handler */
    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } > FLASH

    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);
    } > FLASH

    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } > FLASH

    /* Load address of .data (its initial values live in flash) */
    _sidata = LOADADDR(.data);

    /* Initialised data: runs from RAM, copied from flash at reset */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    /* Zero-initialised data */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } > RAM

    /* Reserve heap + stack so the link fails if RAM is too small */
    ._user_heap_stack (NOLOAD) :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } > RAM

    .ARM.attributes 0 : { *(.ARM.attributes) }
}

/* Stack/heap reservation must fit below the top of RAM */
ASSERT(ADDR(._user_heap_stack) + SIZEOF(._user_heap_stack) <= _estack,
       "Not enough RAM for data + heap + stack - check RAM_SIZE")
