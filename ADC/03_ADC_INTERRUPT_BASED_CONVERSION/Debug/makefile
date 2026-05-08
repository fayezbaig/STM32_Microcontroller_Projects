################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Startup/subdir.mk
-include Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := 03_ADC_INTERRUPT_BASED_CONVERSION
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
03_ADC_INTERRUPT_BASED_CONVERSION.elf \

MAP_FILES += \
03_ADC_INTERRUPT_BASED_CONVERSION.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
03_ADC_INTERRUPT_BASED_CONVERSION.list \


# All Target
all: main-build

# Main-build Target
main-build: 03_ADC_INTERRUPT_BASED_CONVERSION.elf secondary-outputs

# Tool invocations
03_ADC_INTERRUPT_BASED_CONVERSION.elf 03_ADC_INTERRUPT_BASED_CONVERSION.map: $(OBJS) $(USER_OBJS) D:\Embedded\ Sofware\ADC\03_ADC_INTERRUPT_BASED_CONVERSION\STM32F407VGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "03_ADC_INTERRUPT_BASED_CONVERSION.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"D:\Embedded Sofware\ADC\03_ADC_INTERRUPT_BASED_CONVERSION\STM32F407VGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="03_ADC_INTERRUPT_BASED_CONVERSION.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

03_ADC_INTERRUPT_BASED_CONVERSION.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "03_ADC_INTERRUPT_BASED_CONVERSION.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) 03_ADC_INTERRUPT_BASED_CONVERSION.elf 03_ADC_INTERRUPT_BASED_CONVERSION.list 03_ADC_INTERRUPT_BASED_CONVERSION.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
