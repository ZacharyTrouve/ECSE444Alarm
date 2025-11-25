################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a.c \
../Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a_bus.c 

OBJS += \
./Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a.o \
./Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a_bus.o 

C_DEPS += \
./Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a.d \
./Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a_bus.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/B-L4S5I-IOT01A/%.o Drivers/BSP/B-L4S5I-IOT01A/%.su Drivers/BSP/B-L4S5I-IOT01A/%.cyclo: ../Drivers/BSP/B-L4S5I-IOT01A/%.c Drivers/BSP/B-L4S5I-IOT01A/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L4S5xx -c -I../Core/Inc -I"/Users/zachary/Documents/STM32CubeIDE/workspace_1.19.0/project_ble_attempt2/Drivers/Components" -I"/Users/zachary/Documents/STM32CubeIDE/workspace_1.19.0/project_ble_attempt2/Drivers/Components/Common" -I"/Users/zachary/Documents/STM32CubeIDE/workspace_1.19.0/project_ble_attempt2/Drivers/Components/lis3mdl" -I"/Users/zachary/Documents/STM32CubeIDE/workspace_1.19.0/project_ble_attempt2/Drivers/Components/hts221" -I"/Users/zachary/Documents/STM32CubeIDE/workspace_1.19.0/project_ble_attempt2/Drivers/Components/lsm6dsl" -I"/Users/zachary/Documents/STM32CubeIDE/workspace_1.19.0/project_ble_attempt2/Drivers/Components/lps22hb" -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../BlueNRG_MS/App -I../BlueNRG_MS/Target -I../Drivers/BSP/B-L4S5I-IOT01A -I../Middlewares/ST/BlueNRG-MS/utils -I../Middlewares/ST/BlueNRG-MS/includes -I../Middlewares/ST/BlueNRG-MS/hci/hci_tl_patterns/Basic -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-B-2d-L4S5I-2d-IOT01A

clean-Drivers-2f-BSP-2f-B-2d-L4S5I-2d-IOT01A:
	-$(RM) ./Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a.cyclo ./Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a.d ./Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a.o ./Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a.su ./Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a_bus.cyclo ./Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a_bus.d ./Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a_bus.o ./Drivers/BSP/B-L4S5I-IOT01A/b_l4s5i_iot01a_bus.su

.PHONY: clean-Drivers-2f-BSP-2f-B-2d-L4S5I-2d-IOT01A

