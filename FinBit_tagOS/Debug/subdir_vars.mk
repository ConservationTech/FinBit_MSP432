################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CFG_SRCS += \
../FinBit_tagOS.cfg 

CPP_SRCS += \
../MAX30105.cpp \
../i2c_helper.cpp \
../main.cpp \
../system_clock.cpp 

CMD_SRCS += \
../MSP_EXP432P401R.cmd 

C_SRCS += \
../MSP_EXP432P401R.c 

GEN_CMDS += \
./configPkg/linker.cmd 

GEN_FILES += \
./configPkg/linker.cmd \
./configPkg/compiler.opt 

GEN_MISC_DIRS += \
./configPkg/ 

C_DEPS += \
./MSP_EXP432P401R.d 

GEN_OPTS += \
./configPkg/compiler.opt 

OBJS += \
./MAX30105.obj \
./MSP_EXP432P401R.obj \
./i2c_helper.obj \
./main.obj \
./system_clock.obj 

CPP_DEPS += \
./MAX30105.d \
./i2c_helper.d \
./main.d \
./system_clock.d 

GEN_MISC_DIRS__QUOTED += \
"configPkg/" 

OBJS__QUOTED += \
"MAX30105.obj" \
"MSP_EXP432P401R.obj" \
"i2c_helper.obj" \
"main.obj" \
"system_clock.obj" 

C_DEPS__QUOTED += \
"MSP_EXP432P401R.d" 

CPP_DEPS__QUOTED += \
"MAX30105.d" \
"i2c_helper.d" \
"main.d" \
"system_clock.d" 

GEN_FILES__QUOTED += \
"configPkg/linker.cmd" \
"configPkg/compiler.opt" 

CPP_SRCS__QUOTED += \
"../MAX30105.cpp" \
"../i2c_helper.cpp" \
"../main.cpp" \
"../system_clock.cpp" 

C_SRCS__QUOTED += \
"../MSP_EXP432P401R.c" 


