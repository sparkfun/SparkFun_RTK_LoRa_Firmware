################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo.c \
../Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo_radio.c 

OBJS += \
./Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo.o \
./Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo_radio.o 

C_DEPS += \
./Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo.d \
./Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo_radio.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/STM32WLxx_Nucleo/%.o Drivers/STM32WLxx_Nucleo/%.su Drivers/STM32WLxx_Nucleo/%.cyclo: ../Drivers/STM32WLxx_Nucleo/%.c Drivers/STM32WLxx_Nucleo/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WLE5xx -c -I../../Core/Inc -I"/home/paul/Documents/RTCM_TRX_FSS_RTK/STM32CubeIDE/Drivers/STM32WLxx_Nucleo" -I../../SubGHz_Phy/App/include -I../../SubGHz_Phy/Target/include -I../../Utilities/trace/adv_trace -I../../Drivers/STM32WLxx_HAL_Driver/Inc -I../../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../../Utilities/misc -I../../Utilities/timer -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../../Middlewares/Third_Party/SubGHz_Phy -I../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-STM32WLxx_Nucleo

clean-Drivers-2f-STM32WLxx_Nucleo:
	-$(RM) ./Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo.cyclo ./Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo.d ./Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo.o ./Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo.su ./Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo_radio.cyclo ./Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo_radio.d ./Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo_radio.o ./Drivers/STM32WLxx_Nucleo/stm32wlxx_nucleo_radio.su

.PHONY: clean-Drivers-2f-STM32WLxx_Nucleo

