# Makefile - bare-metal build for STM32F401/F411 Black Pill
#
#   make            -> build/firmware.elf, .bin, .hex + size report
#   make flash      -> program via ST-Link (st-flash, from the stlink tools)
#   make clean
#
# Requires: arm-none-eabi-gcc (GNU Arm Embedded Toolchain)

TARGET   := firmware
BUILD    := build

CC       := arm-none-eabi-gcc
OBJCOPY  := arm-none-eabi-objcopy
SIZE     := arm-none-eabi-size

LDSCRIPT := Startup/stm32f4_blackpill.ld
SRCS     := $(wildcard Src/*.c) Startup/startup_stm32f4.c
OBJS     := $(patsubst %.c,$(BUILD)/%.o,$(SRCS))

MCU      := -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

CFLAGS   := $(MCU) -std=c11 -O2 -g3 -Wall -Wextra \
            -ffreestanding -ffunction-sections -fdata-sections \
            -IInc -MMD -MP

LDFLAGS  := $(MCU) -T$(LDSCRIPT) -nostdlib -nostartfiles \
            -Wl,--gc-sections -Wl,-Map=$(BUILD)/$(TARGET).map -Wl,--print-memory-usage

LDLIBS   := -lgcc    # 64-bit division used by the BMP280 pressure formula

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).bin $(BUILD)/$(TARGET).hex
	@$(SIZE) $<

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	$(CC) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

flash: $(BUILD)/$(TARGET).bin
	st-flash --reset write $< 0x08000000

clean:
	rm -rf $(BUILD)

-include $(OBJS:.o=.d)

.PHONY: all flash clean
