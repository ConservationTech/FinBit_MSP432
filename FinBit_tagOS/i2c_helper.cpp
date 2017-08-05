/*
 * i2c_helper.cpp
 *
 *  Created on: Jun 29, 2017
 *      Author: dave
 */

/*
 *
 * DEPRECATED LOW-LEVEL I2C READ AND WRITE FUNCTIONS ARE RETAINED BELOW
 *
 * */

/* XDCtools Header files */
#include <xdc/runtime/System.h>
#include <xdc/runtime/Diags.h>
#include <xdc/std.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>

/* TI-RTOS Header files */
// #include <gpio.h>
// #include <i2c.h>
// #include <msp.h>

/* Include the generic I2C helper function prototypes */
#include "i2c_helper.h"

/* Include system clock for fine-scaling timing on I2C waits */
#include "system_clock.h"

/* Example/Board Header files */
// #include "MSP_EXP432P401R.h"
#include "Board.h"

#include "modules.h"

#ifdef USE_MAX30105
    #include "MAX30105.h"           /* Include the MAX30105 header file */
#endif

///////  readRegisterU8 for SIGNED 8-bit status registers (e.g.: MAX30105 tempFrac value)

int8_t readRegisterS8(I2C_Handle handle, uint8_t devAddr, uint8_t regAddr, int8_t *data, size_t length)
{
    uint8_t             txBuffer[2];
    I2C_Transaction     i2cTransaction;

    i2cTransaction.slaveAddress = devAddr;

    // Write to an 8-bit status register
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.readBuf = data;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readCount = length;

    txBuffer[0] = regAddr;   //HB Addr

    if (!I2C_transfer(handle, &i2cTransaction)) {
        GPIO_write(Board_LED1, Board_LED_OFF);
        GPIO_write(Board_LED2, Board_LED_OFF);
        GPIO_write(Board_LED3, Board_LED_OFF);
        GPIO_write(Board_LED1, Board_LED_ON);
        GPIO_toggle(Board_LED1);
        System_printf("Bad I2C transfer - problem communicating with Board_ADDR %d!\n", devAddr);
        System_flush();
        System_abort("Bailing from i2c_helper.cpp readRegisterS8() function!\n");
    }

    return(*data);

}

///////  readRegisterU8 for UNSIGNED 8-bit status registers (e.g.: MAX30105 tempFrac value)

uint8_t readRegisterU8(I2C_Handle handle, uint8_t devAddr, uint8_t regAddr, uint8_t *data, size_t length)
{
    uint8_t             txBuffer[2];
    I2C_Transaction     i2cTransaction;

    i2cTransaction.slaveAddress = devAddr;

    // Write to an 8-bit status register
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.readBuf = data;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readCount = length;

    txBuffer[0] = regAddr;   //HB Addr

    if (!I2C_transfer(handle, &i2cTransaction)) {
        GPIO_write(Board_LED1, Board_LED_OFF);
        GPIO_write(Board_LED2, Board_LED_OFF);
        GPIO_write(Board_LED3, Board_LED_OFF);
        GPIO_write(Board_LED1, Board_LED_ON);
        GPIO_toggle(Board_LED1);
        System_printf("Bad I2C transfer - problem communicating with Board_ADDR %d!\n", devAddr);
        System_flush();
        System_abort("Bailing from i2c_helper.cpp readRegisterU8() function!\n");
    }

    return(*data);

}

///////  readRegisterU8 for UNSIGNED 16-bit status registers (e.g.: MS5803-14BA PROM read value)

uint16_t readRegisterU16(I2C_Handle handle, uint8_t devAddr, uint8_t regAddr, uint16_t *data16, size_t length)
{
    uint8_t             txBuffer[2];
    I2C_Transaction     i2cTransaction;

    i2cTransaction.slaveAddress = devAddr;

    // Write to an 8-bit status register
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.readBuf = data16;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readCount = length;

    txBuffer[0] = regAddr;   //HB Addr

    if (!I2C_transfer(handle, &i2cTransaction)) {
        GPIO_write(Board_LED1, Board_LED_OFF);
        GPIO_write(Board_LED2, Board_LED_OFF);
        GPIO_write(Board_LED3, Board_LED_OFF);
        GPIO_write(Board_LED1, Board_LED_ON);
        GPIO_toggle(Board_LED1);
        System_printf("Bad I2C transfer - problem communicating with Board_ADDR %d!\n", devAddr);
        System_flush();
        System_abort("Bailing from i2c_helper.cpp readRegisterU16() function!\n");
    }

    return(*data16);

}

///////  readRegisterU8 for UNSIGNED 16-bit status registers (e.g.: MS5803-14BA PROM read value)

float readRegisterUFloat(I2C_Handle handle, uint8_t devAddr, uint8_t regAddr, float *uFloat, size_t length)
{
    uint8_t             txBuffer[2];
    I2C_Transaction     i2cTransaction;

    i2cTransaction.slaveAddress = devAddr;

    // Write to an 8-bit status register
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.readBuf = uFloat;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readCount = length;

    txBuffer[0] = regAddr;   //HB Addr

    if (!I2C_transfer(handle, &i2cTransaction)) {
        GPIO_write(Board_LED1, Board_LED_OFF);
        GPIO_write(Board_LED2, Board_LED_OFF);
        GPIO_write(Board_LED3, Board_LED_OFF);
        GPIO_write(Board_LED1, Board_LED_ON);
        GPIO_toggle(Board_LED1);
        System_printf("Bad I2C transfer - problem communicating with Board_ADDR %d!\n", devAddr);
        System_flush();
        System_abort("Bailing from i2c_helper.cpp readRegisterU16() function!\n");
    }

    return(*uFloat);

}

///////  writeRegisterU8 for UNSIGNED 8-bit status registers (e.g.: MAX30105 tempFrac value)

void writeRegister8(I2C_Handle handle, uint8_t devAddr, uint8_t regAddr, uint8_t value)
{
    uint8_t             txBuffer[2];
    I2C_Transaction     i2cTransaction;

    i2cTransaction.slaveAddress = devAddr;

    // Write to an 8-bit status register
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    txBuffer[0] = regAddr; //  & 0xFF; //LB Addr
    txBuffer[1] = value;   //   & 0xFF;

    if (!I2C_transfer(handle, &i2cTransaction)) {
        GPIO_write(Board_LED1, Board_LED_OFF);
        GPIO_write(Board_LED2, Board_LED_OFF);
        GPIO_write(Board_LED3, Board_LED_OFF);
        GPIO_write(Board_LED1, Board_LED_ON);
        GPIO_toggle(Board_LED1);
        System_printf("Bad I2C transfer - problem communicating with Board_ADDR %d!\n", devAddr);
        System_flush();
        System_abort("Bailing from i2c_helper.cpp writeRegister8() function!\n");
    }

}

///////  writeRegisterU8 for UNSIGNED 8-bit status registers (e.g.: MAX30105 tempFrac value)

void writeRegister8R(I2C_Handle handle, uint8_t devAddr, uint8_t regAddr, uint8_t value, uint8_t readReg)
{
    uint8_t             txBuffer[2], rxBuffer[2];
    I2C_Transaction     i2cTransaction;

    i2cTransaction.slaveAddress = devAddr;

    // Write to an 8-bit status register
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.readCount = 1;

    txBuffer[0] = regAddr;
    txBuffer[1] = value;
    rxBuffer[0] = readReg;

    if (!I2C_transfer(handle, &i2cTransaction)) {
        GPIO_write(Board_LED1, Board_LED_OFF);
        GPIO_write(Board_LED2, Board_LED_OFF);
        GPIO_write(Board_LED3, Board_LED_OFF);
        GPIO_write(Board_LED1, Board_LED_ON);
        GPIO_toggle(Board_LED1);
        System_printf("Bad I2C transfer - problem communicating with Board_ADDR %d!\n", devAddr);
        System_flush();
        System_abort("Bailing from i2c_helper.cpp writeRegister8R() function!\n");
    }

}

/* Migrate sensor aliveness and POST test routines to here, to clean up main taskFxn */

uint8_t runSensorChecks(I2C_Handle i2c) {

    uint8_t badSensors = 0;
    uint8_t partID = 0x00;

    System_printf("Beginning sensor aliveness checks and POSTs (if available)...\n");
    System_flush();

    /* Check aliveness of BNO055 sensor package */

    //    partID = readRegisterU8(i2c, Board_BNO055_ADDR, 0, &partID, 1);
    //    if (partID == 0xA0) {
    //        if (POSTPRINT) {
    //            System_printf("BNO055 PartID = %d. That's the expected WHOAMI value for this 9-DOF sensor!\n", partID);
    //            System_flush();
    //        }
    //
    //        // Run BNO055 system status and/or power-on self test routines
    //
    //        uint8_t statusID = readRegisterU8(i2c, Board_BNO055_ADDR, 0x39, &statusID, 1);
    //        if (POSTPRINT){
    //            System_printf("\tBNO055 status = %d\n", statusID);
    //            System_flush();
    //        }
    //
    //
    //    } else {
    //        if (POSTPRINT) {
    //            System_printf("PartID = %d. That's not the expected WHOAMI value for BNO055!\n", partID);
    //            System_flush();
    //        }
    //    }

    /* Check aliveness of MAX30105 */

    partID = readRegisterU8(i2c, Board_MAX30105_ADDR, 0xFF, &partID, 1);
    Task_sleep(1);
    if (partID == 0x15) {
        System_printf("\tMAX30105: alive!\n");
        System_flush();
        badSensors += 0;
    } else {
        badSensors += 1;
        System_printf("\tPartID = %d. That's not the expected WHOAMI values for this RPOX sensor!\n", partID);
        System_flush();
    }

    /* Check aliveness of MPU-9250 sensor package */

    partID = readRegisterU8(i2c, Board_MPU9250_ADDR, 117, &partID, 1);
    if (partID == 0x71) {
        System_printf("\tMPU-9250: alive!\n");
        System_flush();
        badSensors += 0;
    } else {
        badSensors += 1;
        System_printf("\tPartID = %d. That's not the expected WHOAMI value for MPU-9250!\n", partID);
        System_flush();
    }


    /* Check aliveness of MCP9808 sensor package */

    partID = readRegisterU8(i2c, Board_MCP9808_ADDR, 0x07, &partID, 1);
    if (partID == 0x04) {
        System_printf("\tMCP9808: alive!\n");
        System_flush();
        badSensors += 0;
    } else {
        badSensors += 1;
        System_printf("\tPartID = %d. That's not the expected WHOAMI value for MCP9808!\n", partID);
        System_flush();
    }


    /* Check aliveness of MS5803-14BA sensor package */
    writeRegister8(i2c, Board_MS5803_14BA_ADDR, 0x1E, 1);

    Task_sleep(10);

    uint16_t promVal = readRegisterU16(i2c, Board_MS5803_14BA_ADDR, 0xA0, &promVal, 1);

    if (promVal >= 0x01) {
        System_printf("\tMS5803-14BA: alive!\n");
        System_flush();
        badSensors += 0;
    } else {
        badSensors += 1;
        System_printf("\tPartID = %d. That's a weird result for a proxy WHOAMI value for MS5803-14BA!\n", partID);
        System_flush();
    }

    return(badSensors);

}
