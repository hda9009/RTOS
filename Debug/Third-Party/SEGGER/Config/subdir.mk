################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Third-Party/SEGGER/Config/SEGGER_SYSVIEW_Config_FreeRTOS.c 

OBJS += \
./Third-Party/SEGGER/Config/SEGGER_SYSVIEW_Config_FreeRTOS.o 

C_DEPS += \
./Third-Party/SEGGER/Config/SEGGER_SYSVIEW_Config_FreeRTOS.d 


# Each subdirectory must supply rules for building sources it contributes
Third-Party/SEGGER/Config/%.o: ../Third-Party/SEGGER/Config/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DSTM32 -DUSE_SEMIHOSTING -DSTM32F4 -DSTM32F446RETx -DNUCLEO_F446RE -DDEBUG -DSTM32F446xx -DUSE_STDPERIPH_DRIVER -I"D:/Project files/STM Workspace/RTOS_Project/Third-Party/SEGGER/SEGGER" -I"D:/Project files/STM Workspace/RTOS_Project/Third-Party/FreeRTOS/org/Source/portable/GCC/ARM_CM4F" -I"D:/Project files/STM Workspace/RTOS_Project/Third-Party/SEGGER/Config" -I"D:/Project files/STM Workspace/RTOS_Project/Third-Party/SEGGER/OS" -I"D:/Project files/STM Workspace/RTOS_Project/Third-Party/FreeRTOS/org/Source/include" -I"D:/Installed Programs/cygwin/usr/include/bash" -I"D:/Project files/STM Workspace/RTOS_Project/config" -I"D:/Project files/STM Workspace/RTOS_Project/StdPeriph_Driver/inc" -I"D:/Project files/STM Workspace/RTOS_Project/Third-Party/FreeRTOS/org/Source/portable/GCC/ARM_CM4F" -I"D:/Project files/STM Workspace/RTOS_Project/inc" -I"D:/Project files/STM Workspace/RTOS_Project/CMSIS/device" -I"D:/Project files/STM Workspace/RTOS_Project/CMSIS/core" -O0 -g3 -Wall -fmessage-length=0 -ffunction-sections -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


