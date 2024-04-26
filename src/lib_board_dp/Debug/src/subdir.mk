################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/board.c 

C_DEPS += \
./src/board.d 

OBJS += \
./src/board.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=c99 -D__REDLIB__ -DDEBUG -D__CODE_RED -DCORE_M0PLUS -I"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\lib_chip_nss" -I"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\lib_board_dp\inc" -I"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\lib_board_dp\mods" -I"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\lib_chip_nss\inc" -I"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\lib_chip_nss\mods" -I"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\mods" -include"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\lib_chip_nss\mods\chip_sel.h" -include"C:\Users\archu\Documents\MCUXpressoIDE_11.7.1_9221\workspace\lib_board_dp\mods\board_sel.h" -Og -g3 -pedantic -Wall -Wextra -Wconversion -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/board.d ./src/board.o

.PHONY: clean-src

