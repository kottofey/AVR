################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
Everything.lss \

FLASH_IMAGE += \
Everything.hex \

EEPROM_IMAGE += \
Everything.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: Everything.elf secondary-outputs

# Tool invocations
Everything.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,Everything.map -mmcu=atmega16 -o "Everything.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

Everything.lss: Everything.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S Everything.elf  >"Everything.lss"
	@echo 'Finished building: $@'
	@echo ' '

Everything.hex: Everything.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -R .fuse -R .lock -R .signature -O ihex Everything.elf  "Everything.hex"
	@echo 'Finished building: $@'
	@echo ' '

Everything.eep: Everything.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex Everything.elf  "Everything.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: Everything.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega16 Everything.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(EEPROM_IMAGE)$(FLASH_IMAGE)$(ELFS)$(LSS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS) Everything.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
