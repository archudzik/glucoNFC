################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/adcdac_nss.c \
../src/bussync_nss.c \
../src/clock_nss.c \
../src/eeprom_nss.c \
../src/flash_nss.c \
../src/gpio_nss.c \
../src/i2c_nss.c \
../src/i2d_nss.c \
../src/iap_nss.c \
../src/iocon_nss.c \
../src/nfc_nss.c \
../src/pmu_nss.c \
../src/rtc_nss.c \
../src/ssp_nss.c \
../src/syscon_nss.c \
../src/timer_nss.c \
../src/tsen_nss.c \
../src/wwdt_nss.c 

C_DEPS += \
./src/adcdac_nss.d \
./src/bussync_nss.d \
./src/clock_nss.d \
./src/eeprom_nss.d \
./src/flash_nss.d \
./src/gpio_nss.d \
./src/i2c_nss.d \
./src/i2d_nss.d \
./src/iap_nss.d \
./src/iocon_nss.d \
./src/nfc_nss.d \
./src/pmu_nss.d \
./src/rtc_nss.d \
./src/ssp_nss.d \
./src/syscon_nss.d \
./src/timer_nss.d \
./src/tsen_nss.d \
./src/wwdt_nss.d 

OBJS += \
./src/adcdac_nss.o \
./src/bussync_nss.o \
./src/clock_nss.o \
./src/eeprom_nss.o \
./src/flash_nss.o \
./src/gpio_nss.o \
./src/i2c_nss.o \
./src/i2d_nss.o \
./src/iap_nss.o \
./src/iocon_nss.o \
./src/nfc_nss.o \
./src/pmu_nss.o \
./src/rtc_nss.o \
./src/ssp_nss.o \
./src/syscon_nss.o \
./src/timer_nss.o \
./src/tsen_nss.o \
./src/wwdt_nss.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=c99 -D__REDLIB__ -DNDEBUG -D__CODE_RED -DCORE_M0PLUS -I"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\lib_chip_nss\inc" -I"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\lib_chip_nss\mods" -I"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\mods" -include"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\lib_chip_nss\mods\chip_sel.h" -Os -pedantic -Wall -Wextra -Wconversion -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/adcdac_nss.d ./src/adcdac_nss.o ./src/bussync_nss.d ./src/bussync_nss.o ./src/clock_nss.d ./src/clock_nss.o ./src/eeprom_nss.d ./src/eeprom_nss.o ./src/flash_nss.d ./src/flash_nss.o ./src/gpio_nss.d ./src/gpio_nss.o ./src/i2c_nss.d ./src/i2c_nss.o ./src/i2d_nss.d ./src/i2d_nss.o ./src/iap_nss.d ./src/iap_nss.o ./src/iocon_nss.d ./src/iocon_nss.o ./src/nfc_nss.d ./src/nfc_nss.o ./src/pmu_nss.d ./src/pmu_nss.o ./src/rtc_nss.d ./src/rtc_nss.o ./src/ssp_nss.d ./src/ssp_nss.o ./src/syscon_nss.d ./src/syscon_nss.o ./src/timer_nss.d ./src/timer_nss.o ./src/tsen_nss.d ./src/tsen_nss.o ./src/wwdt_nss.d ./src/wwdt_nss.o

.PHONY: clean-src

