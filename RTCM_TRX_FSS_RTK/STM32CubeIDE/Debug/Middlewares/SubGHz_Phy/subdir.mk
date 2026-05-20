################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/paul/Documents/RTCM_TRX_FSS_RTK/Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/lr_fhss_mac.c \
/home/paul/Documents/RTCM_TRX_FSS_RTK/Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio.c \
/home/paul/Documents/RTCM_TRX_FSS_RTK/Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_driver.c \
/home/paul/Documents/RTCM_TRX_FSS_RTK/Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_fw.c \
/home/paul/Documents/RTCM_TRX_FSS_RTK/Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/wl_lr_fhss.c 

OBJS += \
./Middlewares/SubGHz_Phy/lr_fhss_mac.o \
./Middlewares/SubGHz_Phy/radio.o \
./Middlewares/SubGHz_Phy/radio_driver.o \
./Middlewares/SubGHz_Phy/radio_fw.o \
./Middlewares/SubGHz_Phy/wl_lr_fhss.o 

C_DEPS += \
./Middlewares/SubGHz_Phy/lr_fhss_mac.d \
./Middlewares/SubGHz_Phy/radio.d \
./Middlewares/SubGHz_Phy/radio_driver.d \
./Middlewares/SubGHz_Phy/radio_fw.d \
./Middlewares/SubGHz_Phy/wl_lr_fhss.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/SubGHz_Phy/lr_fhss_mac.o: /home/paul/Documents/RTCM_TRX_FSS_RTK/Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/lr_fhss_mac.c Middlewares/SubGHz_Phy/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WLE5xx -c -I../../Core/Inc -I"/home/paul/Documents/RTCM_TRX_FSS_RTK/STM32CubeIDE/Drivers/STM32WLxx_Nucleo" -I../../SubGHz_Phy/App/include -I../../SubGHz_Phy/Target/include -I../../Utilities/trace/adv_trace -I../../Drivers/STM32WLxx_HAL_Driver/Inc -I../../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../../Utilities/misc -I../../Utilities/timer -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../../Middlewares/Third_Party/SubGHz_Phy -I../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Middlewares/SubGHz_Phy/radio.o: /home/paul/Documents/RTCM_TRX_FSS_RTK/Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio.c Middlewares/SubGHz_Phy/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WLE5xx -c -I../../Core/Inc -I"/home/paul/Documents/RTCM_TRX_FSS_RTK/STM32CubeIDE/Drivers/STM32WLxx_Nucleo" -I../../SubGHz_Phy/App/include -I../../SubGHz_Phy/Target/include -I../../Utilities/trace/adv_trace -I../../Drivers/STM32WLxx_HAL_Driver/Inc -I../../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../../Utilities/misc -I../../Utilities/timer -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../../Middlewares/Third_Party/SubGHz_Phy -I../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Middlewares/SubGHz_Phy/radio_driver.o: /home/paul/Documents/RTCM_TRX_FSS_RTK/Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_driver.c Middlewares/SubGHz_Phy/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WLE5xx -c -I../../Core/Inc -I"/home/paul/Documents/RTCM_TRX_FSS_RTK/STM32CubeIDE/Drivers/STM32WLxx_Nucleo" -I../../SubGHz_Phy/App/include -I../../SubGHz_Phy/Target/include -I../../Utilities/trace/adv_trace -I../../Drivers/STM32WLxx_HAL_Driver/Inc -I../../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../../Utilities/misc -I../../Utilities/timer -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../../Middlewares/Third_Party/SubGHz_Phy -I../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Middlewares/SubGHz_Phy/radio_fw.o: /home/paul/Documents/RTCM_TRX_FSS_RTK/Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_fw.c Middlewares/SubGHz_Phy/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WLE5xx -c -I../../Core/Inc -I"/home/paul/Documents/RTCM_TRX_FSS_RTK/STM32CubeIDE/Drivers/STM32WLxx_Nucleo" -I../../SubGHz_Phy/App/include -I../../SubGHz_Phy/Target/include -I../../Utilities/trace/adv_trace -I../../Drivers/STM32WLxx_HAL_Driver/Inc -I../../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../../Utilities/misc -I../../Utilities/timer -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../../Middlewares/Third_Party/SubGHz_Phy -I../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Middlewares/SubGHz_Phy/wl_lr_fhss.o: /home/paul/Documents/RTCM_TRX_FSS_RTK/Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/wl_lr_fhss.c Middlewares/SubGHz_Phy/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DCORE_CM4 -DUSE_HAL_DRIVER -DSTM32WLE5xx -c -I../../Core/Inc -I"/home/paul/Documents/RTCM_TRX_FSS_RTK/STM32CubeIDE/Drivers/STM32WLxx_Nucleo" -I../../SubGHz_Phy/App/include -I../../SubGHz_Phy/Target/include -I../../Utilities/trace/adv_trace -I../../Drivers/STM32WLxx_HAL_Driver/Inc -I../../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy -I../../Utilities/misc -I../../Utilities/timer -I../../Utilities/lpm/tiny_lpm -I../../Middlewares/Third_Party/FreeRTOS/Source/include -I../../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../../Drivers/CMSIS/Device/ST/STM32WLxx/Include -I../../Middlewares/Third_Party/SubGHz_Phy -I../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Middlewares-2f-SubGHz_Phy

clean-Middlewares-2f-SubGHz_Phy:
	-$(RM) ./Middlewares/SubGHz_Phy/lr_fhss_mac.cyclo ./Middlewares/SubGHz_Phy/lr_fhss_mac.d ./Middlewares/SubGHz_Phy/lr_fhss_mac.o ./Middlewares/SubGHz_Phy/lr_fhss_mac.su ./Middlewares/SubGHz_Phy/radio.cyclo ./Middlewares/SubGHz_Phy/radio.d ./Middlewares/SubGHz_Phy/radio.o ./Middlewares/SubGHz_Phy/radio.su ./Middlewares/SubGHz_Phy/radio_driver.cyclo ./Middlewares/SubGHz_Phy/radio_driver.d ./Middlewares/SubGHz_Phy/radio_driver.o ./Middlewares/SubGHz_Phy/radio_driver.su ./Middlewares/SubGHz_Phy/radio_fw.cyclo ./Middlewares/SubGHz_Phy/radio_fw.d ./Middlewares/SubGHz_Phy/radio_fw.o ./Middlewares/SubGHz_Phy/radio_fw.su ./Middlewares/SubGHz_Phy/wl_lr_fhss.cyclo ./Middlewares/SubGHz_Phy/wl_lr_fhss.d ./Middlewares/SubGHz_Phy/wl_lr_fhss.o ./Middlewares/SubGHz_Phy/wl_lr_fhss.su

.PHONY: clean-Middlewares-2f-SubGHz_Phy

