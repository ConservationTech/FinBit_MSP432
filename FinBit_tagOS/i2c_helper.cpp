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

/* Example/Board Header files */
// #include "MSP_EXP432P401R.h"
#include "Board.h"

/* Include the MAX30105 header file */
#include "MAX30105.h"

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
        GPIO_write(Board_LED1, Board_LED_ON);
        System_abort("Bad I2C transfer!");
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
        GPIO_write(Board_LED1, Board_LED_ON);
        System_abort("Bad I2C transfer!");
    }

    return(*data);

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
        GPIO_write(Board_LED1, Board_LED_ON);
        System_abort("Bad I2C transfer!");
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
        GPIO_write(Board_LED1, Board_LED_ON);
        System_abort("Bad I2C transfer!");
    }
}


