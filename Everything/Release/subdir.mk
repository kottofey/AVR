################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../DS18B20.c \
../LCD.c \
../Messages.c \
../Timers.c \
../keyboard.c \
../main.c \
../menu.c \
../my_uart.c \
../somthing.c 

OBJS += \
./DS18B20.o \
./LCD.o \
./Messages.o \
./Timers.o \
./keyboard.o \
./main.o \
./menu.o \
./my_uart.o \
./somthing.o 

C_DEPS += \
./DS18B20.d \
./LCD.d \
./Messages.d \
./Timers.d \
./keyboard.d \
./main.d \
./menu.d \
./my_uart.d \
./somthing.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega16 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.o: ../main.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -g2 -gstabs -Os -fpack-struct -fshort-enums -ffunction-sections -fdata-sections -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=atmega16 -MMD -MP -MF"$(@:%.o=%.d)" -MT"main.d" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


