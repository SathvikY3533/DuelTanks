################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
ADC1.o: C:/Users/pani_/OneDrive/Desktop/UT\ Stuff/Freshman\ Spring\ Classes/319H/MSPM0_ValvanoWare/inc/ADC1.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/TI/ccs2040/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug" -I"C:/TI/mspm0_sdk_2_06_00_05/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_06_00_05/source" -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"ADC1.d_raw" -MT"ADC1.o" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Clock.o: C:/Users/pani_/OneDrive/Desktop/UT\ Stuff/Freshman\ Spring\ Classes/319H/MSPM0_ValvanoWare/inc/Clock.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/TI/ccs2040/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug" -I"C:/TI/mspm0_sdk_2_06_00_05/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_06_00_05/source" -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"Clock.d_raw" -MT"Clock.o" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/TI/ccs2040/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug" -I"C:/TI/mspm0_sdk_2_06_00_05/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_06_00_05/source" -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

LaunchPad.o: C:/Users/pani_/OneDrive/Desktop/UT\ Stuff/Freshman\ Spring\ Classes/319H/MSPM0_ValvanoWare/inc/LaunchPad.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/TI/ccs2040/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug" -I"C:/TI/mspm0_sdk_2_06_00_05/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_06_00_05/source" -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"LaunchPad.d_raw" -MT"LaunchPad.o" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

SPI.o: C:/Users/pani_/OneDrive/Desktop/UT\ Stuff/Freshman\ Spring\ Classes/319H/MSPM0_ValvanoWare/inc/SPI.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/TI/ccs2040/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug" -I"C:/TI/mspm0_sdk_2_06_00_05/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_06_00_05/source" -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"SPI.d_raw" -MT"SPI.o" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

ST7735.o: C:/Users/pani_/OneDrive/Desktop/UT\ Stuff/Freshman\ Spring\ Classes/319H/MSPM0_ValvanoWare/inc/ST7735.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/TI/ccs2040/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug" -I"C:/TI/mspm0_sdk_2_06_00_05/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_06_00_05/source" -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"ST7735.d_raw" -MT"ST7735.o" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

TExaS.o: C:/Users/pani_/OneDrive/Desktop/UT\ Stuff/Freshman\ Spring\ Classes/319H/MSPM0_ValvanoWare/inc/TExaS.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/TI/ccs2040/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug" -I"C:/TI/mspm0_sdk_2_06_00_05/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_06_00_05/source" -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"TExaS.d_raw" -MT"TExaS.o" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

Timer.o: C:/Users/pani_/OneDrive/Desktop/UT\ Stuff/Freshman\ Spring\ Classes/319H/MSPM0_ValvanoWare/inc/Timer.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/TI/ccs2040/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug" -I"C:/TI/mspm0_sdk_2_06_00_05/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_06_00_05/source" -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"Timer.d_raw" -MT"Timer.o" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/ECE319K_Lab7/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


