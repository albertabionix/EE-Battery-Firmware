################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BMS/Src/bms_app.c \
../BMS/Src/bms_i2c.c \
../BMS/Src/bms_thresholds.c \
../BMS/Src/ltc4162.c \
../BMS/Src/max17320.c \
../BMS/Src/telemetry.c 

OBJS += \
./BMS/Src/bms_app.o \
./BMS/Src/bms_i2c.o \
./BMS/Src/bms_thresholds.o \
./BMS/Src/ltc4162.o \
./BMS/Src/max17320.o \
./BMS/Src/telemetry.o 

C_DEPS += \
./BMS/Src/bms_app.d \
./BMS/Src/bms_i2c.d \
./BMS/Src/bms_thresholds.d \
./BMS/Src/ltc4162.d \
./BMS/Src/max17320.d \
./BMS/Src/telemetry.d 


# Each subdirectory must supply rules for building sources it contributes
BMS/Src/%.o BMS/Src/%.su BMS/Src/%.cyclo: ../BMS/Src/%.c BMS/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G030xx -c -I../Core/Inc -I../BMS/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc -I../Drivers/STM32G0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-BMS-2f-Src

clean-BMS-2f-Src:
	-$(RM) ./BMS/Src/bms_app.cyclo ./BMS/Src/bms_app.d ./BMS/Src/bms_app.o ./BMS/Src/bms_app.su ./BMS/Src/bms_i2c.cyclo ./BMS/Src/bms_i2c.d ./BMS/Src/bms_i2c.o ./BMS/Src/bms_i2c.su ./BMS/Src/bms_thresholds.cyclo ./BMS/Src/bms_thresholds.d ./BMS/Src/bms_thresholds.o ./BMS/Src/bms_thresholds.su ./BMS/Src/ltc4162.cyclo ./BMS/Src/ltc4162.d ./BMS/Src/ltc4162.o ./BMS/Src/ltc4162.su ./BMS/Src/max17320.cyclo ./BMS/Src/max17320.d ./BMS/Src/max17320.o ./BMS/Src/max17320.su ./BMS/Src/telemetry.cyclo ./BMS/Src/telemetry.d ./BMS/Src/telemetry.o ./BMS/Src/telemetry.su

.PHONY: clean-BMS-2f-Src

