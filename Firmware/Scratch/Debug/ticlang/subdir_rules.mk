################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
ticlang/%.o: ../ticlang/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/TI/ccs2040/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/Scratch" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/Scratch/Debug" -I"C:/TI/mspm0_sdk_2_06_00_05/source/third_party/CMSIS/Core/Include" -I"C:/TI/mspm0_sdk_2_06_00_05/source" -D__MSPM0G3507__ -gdwarf-3 -MMD -MP -MF"ticlang/$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/pani_/OneDrive/Desktop/UT Stuff/Freshman Spring Classes/319H/MSPM0_ValvanoWare/Scratch/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


