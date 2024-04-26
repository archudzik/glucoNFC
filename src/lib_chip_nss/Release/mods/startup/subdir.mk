################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/archu/Documents/MCUXpressoIDE_11.7.1_9221/workspace/mods/startup/startup.c 

C_DEPS += \
./mods/startup/startup.d 

OBJS += \
./mods/startup/startup.o 


# Each subdirectory must supply rules for building sources it contributes
mods/startup/startup.o: C:/Users/archu/Documents/MCUXpressoIDE_11.7.1_9221/workspace/mods/startup/startup.c mods/startup/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=c99 -D__REDLIB__ -DNDEBUG -D__CODE_RED -DCORE_M0PLUS -I"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\lib_chip_nss\inc" -I"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\lib_chip_nss\mods" -I"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\mods" -include"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\lib_chip_nss\mods\chip_sel.h" -Os -pedantic -Wall -Wextra -Wconversion -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-mods-2f-startup

clean-mods-2f-startup:
	-$(RM) ./mods/startup/startup.d ./mods/startup/startup.o

.PHONY: clean-mods-2f-startup

