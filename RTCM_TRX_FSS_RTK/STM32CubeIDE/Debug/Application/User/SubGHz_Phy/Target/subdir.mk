################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/paul/Documents/RTCM_TRX_FSS_RTK/SubGHz_Phy/Target/radio_board_if.c 

OBJS += \
./Application/User/SubGHz_Phy/Target/radio_board_if.o 

C_DEPS += \
./Application/User/SubGHz_Phy/Target/radio_board_if.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/SubGHz_Phy/Target/radio_board_if.o: /home/paul/Documents/RTCM_TRX_FSS_RTK/SubGHz_Phy/Target/radio_board_if.c Application/User/SubGHz_Phy/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WLE5xx -c -I../../Core/Inc -I"/home/paul/Documents/RTCM_TRX_FSS_RTK/STM32CubeIDE/Drivers/STM32WLxx_Nucleo" -I../../SubGHz_Phy/App/include -I../../SubGHz_Phy/Target/include -I../../Utilities/trace/adv_trace -I../../Drivers/STM32WLxx_HAL_Driver/Inc -I../../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../../Utilities/misc -I../../Utilities/timer -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../../Middlewares/Third_Party/SubGHz_Phy -I../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Application-2f-User-2f-SubGHz_Phy-2f-Target

clean-Application-2f-User-2f-SubGHz_Phy-2f-Target:
	-$(RM) ./Application/User/SubGHz_Phy/Target/radio_board_if.cyclo ./Application/User/SubGHz_Phy/Target/radio_board_if.d ./Application/User/SubGHz_Phy/Target/radio_board_if.o ./Application/User/SubGHz_Phy/Target/radio_board_if.su

.PHONY: clean-Application-2f-User-2f-SubGHz_Phy-2f-Target

