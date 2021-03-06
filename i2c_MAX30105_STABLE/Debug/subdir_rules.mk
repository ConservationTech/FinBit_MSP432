################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
build-880818367:
	@$(MAKE) -Onone -f subdir_rules.mk build-880818367-inproc

build-880818367-inproc: ../MAX30105.cfg
	@echo 'Building file: $<'
	@echo 'Invoking: XDCtools'
	"/Users/dave/ti/xdctools_3_32_00_06_core/xs" --xdcpath="/Users/dave/ti/tirtos_msp43x_2_20_00_06/packages;/Users/dave/ti/tirtos_msp43x_2_20_00_06/products/bios_6_46_00_23/packages;/Users/dave/ti/tirtos_msp43x_2_20_00_06/products/tidrivers_msp43x_2_20_00_08/packages;/Users/dave/ti/tirtos_msp43x_2_20_00_06/products/uia_2_00_06_52/packages;/Applications/ti/ccsv7/ccs_base;" xdc.tools.configuro -o configPkg -t ti.targets.arm.elf.M4F -p ti.platforms.msp432:MSP432P401R -r release -c "/Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.1.LTS" --compileOptions "-mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --fp_mode=strict --include_path=\"/Users/dave/Documents/GitHub/FinBit_MSP432/i2c_MAX30105_STABLE\" --include_path=\"/Users/dave/Documents/GitHub/FinBit_MSP432/i2c_MAX30105_STABLE\" --include_path=\"/Users/dave/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/inc/CMSIS\" --include_path=\"/Users/dave/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/inc\" --include_path=\"/Users/dave/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/driverlib/MSP432P4xx\" --include_path=\"/Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.1.LTS/include\" --include_path=\"/Applications/ti/ccsv7/ccs_base/arm/include\" --include_path=\"/Applications/ti/ccsv7/ccs_base/arm/include/CMSIS\" --advice:power=\"all\" --advice:power_severity=suppress --define=__MSP432P401R__ --define=ccs --define=MSP432WARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi  " "$<"
	@echo 'Finished building: $<'
	@echo ' '

configPkg/linker.cmd: build-880818367 ../MAX30105.cfg
configPkg/compiler.opt: build-880818367
configPkg/: build-880818367

MAX30105.obj: ../MAX30105.cpp $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"/Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --fp_mode=strict --include_path="/Users/dave/Documents/GitHub/FinBit_MSP432/i2c_MAX30105_STABLE" --include_path="/Users/dave/Documents/GitHub/FinBit_MSP432/i2c_MAX30105_STABLE" --include_path="/Users/dave/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/inc/CMSIS" --include_path="/Users/dave/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/inc" --include_path="/Users/dave/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/driverlib/MSP432P4xx" --include_path="/Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.1.LTS/include" --include_path="/Applications/ti/ccsv7/ccs_base/arm/include" --include_path="/Applications/ti/ccsv7/ccs_base/arm/include/CMSIS" --advice:power="all" --advice:power_severity=suppress --define=__MSP432P401R__ --define=ccs --define=MSP432WARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="MAX30105.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

MSP_EXP432P401R.obj: ../MSP_EXP432P401R.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"/Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.1.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --fp_mode=strict --include_path="/Users/dave/Documents/GitHub/FinBit_MSP432/i2c_MAX30105_STABLE" --include_path="/Users/dave/Documents/GitHub/FinBit_MSP432/i2c_MAX30105_STABLE" --include_path="/Users/dave/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/inc/CMSIS" --include_path="/Users/dave/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/inc" --include_path="/Users/dave/ti/tirtos_msp43x_2_20_00_06/products/msp432_driverlib_3_21_00_05/driverlib/MSP432P4xx" --include_path="/Applications/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.1.LTS/include" --include_path="/Applications/ti/ccsv7/ccs_base/arm/include" --include_path="/Applications/ti/ccsv7/ccs_base/arm/include/CMSIS" --advice:power="all" --advice:power_severity=suppress --define=__MSP432P401R__ --define=ccs --define=MSP432WARE -g --gcc --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="MSP_EXP432P401R.d" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


