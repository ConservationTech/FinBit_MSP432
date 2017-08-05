/*
 * MCP9808.cpp
 *
 *  Created on: Jul 10, 2017
 *      Author: dave
 */

/**************** MODIFIED FROM THIS ORIGINAL CODE ***********************/
/*!
    @file     MCP9808.cpp
    @author   K.Townsend (Adafruit Industries)
    @license  BSD (see license.txt)

    I2C Driver for Microchip's MCP9808 I2C Temp sensor

    This is a library for the Adafruit MCP9808 breakout
    ----> http://www.adafruit.com/products/1782

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0 - First release
*/
/**************************************************************************/

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>

#include "Board.h"

/* Include system_clock.h for clock and time functions */
#include "system_clock.h"

#include "MCP9808.h"

#include "i2c_helper.h"
#include "interfaces.h"
#include "modules.h"
#include "condPrintf.h"

// #define DEBUGPRINT          true

/**************************************************************************/
/*!
    @brief  Instantiates a new MCP9808 class
*/
/**************************************************************************/
MCP9808::MCP9808() {
}

/**************************************************************************/
/*!
    @brief  Setups the HW
*/
/**************************************************************************/
bool MCP9808::begin(I2C_Handle i2c, uint8_t regAddr) {

    bool returnFlag = false;

    if (i2c == NULL) {

        if (debugPrint) {
            System_abort("Error Initializing I2C inside MCP9808 class. That's a problem!\n");
            System_flush();
            returnFlag = false;
        }


    } else if (read16(i2c, MCP9808_REG_MANUF_ID) != 0x0054) {

        if (debugPrint) {
            System_abort("Bad manufacturer ID from MCP9808 class. That's a problem!\n");
            System_flush();
            returnFlag = false;
        }

    } else if (read16(i2c, MCP9808_REG_DEVICE_ID) != 0x0400) {

        if (debugPrint) {
            System_abort("Bad device ID from MCP9808 class. That's a problem!\n");
            System_flush();
            returnFlag = false;
        }

    } else {

        // Skip - but add conditional to make sure there's graceful failure if !MCP9808
        // if (!readPartID(i2c) == MCP9808_REG_DEVICE_ID) {
          // Error -- Part ID read from MAX30105 does not match expected part ID.
          // This may mean there is a physical connectivity problem (broken wire, unpowered, etc).

        if (debugPrint) {
            System_printf("Opening communication with MCP9808... time to gather some data!\n");
            System_flush();

            write16(i2c, MCP9808_REG_CONFIG, 0x0);
            returnFlag = true;

        }

    }

    return(returnFlag);

}






/**************************************************************************/
/*!
    @brief  Reads the 16-bit temperature register and returns the Centigrade
            temperature as a float.

*/
/**************************************************************************/
float MCP9808::readTempC( I2C_Handle i2c )
{
  uint16_t t = read16(i2c, MCP9808_REG_AMBIENT_TEMP);

  float temp = t & 0x0FFF;
  temp /=  16.0;
  if (t & 0x1000) temp -= 256;

  return temp;
}



//*************************************************************************
// Set Sensor to Shutdown-State or wake up (Conf_Register BIT8)
// 1= shutdown / 0= wake up
//*************************************************************************

void MCP9808::shutdown_wake(  I2C_Handle i2c, uint8_t sw_ID )
{
    uint16_t conf_shutdown ;
    uint16_t conf_register = read16(i2c, MCP9808_REG_CONFIG);
    if (sw_ID == 1)
    {
       conf_shutdown = conf_register | MCP9808_REG_CONFIG_SHUTDOWN ;
       write16(i2c, MCP9808_REG_CONFIG, conf_shutdown);
    }
    if (sw_ID == 0)
    {
       conf_shutdown = conf_register & ~MCP9808_REG_CONFIG_SHUTDOWN ;
       write16(i2c, MCP9808_REG_CONFIG, conf_shutdown);
    }
}

void MCP9808::shutdown(I2C_Handle i2c )
{
  shutdown_wake(i2c, 1);
}

void MCP9808::wake(I2C_Handle i2c )
{
  shutdown_wake(i2c, 0);
  // delay(250);
  Task_sleep(25);
}



/**************************************************************************/
/*!
    @brief  Low level 16 bit read and write procedures!
*/
/**************************************************************************/

void MCP9808::write16(I2C_Handle i2c, uint8_t regAddr, uint16_t value) {


//    Wire.beginTransmission(_i2caddr);
//    Wire.write((uint8_t)reg);
//    Wire.write(value >> 8);
//    Wire.write(value & 0xFF);
//    Wire.endTransmission();

    uint8_t             txBuffer[3];
    I2C_Transaction     i2cTransaction;

    i2cTransaction.slaveAddress = Board_MCP9808_ADDR;

    // Write to an 8-bit status register
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    txBuffer[0] = regAddr;
    txBuffer[1] = value >> 8;
    txBuffer[2] = value & 0xff;

    if (!I2C_transfer(i2c, &i2cTransaction)) {
        GPIO_write(Board_LED1, Board_LED_ON);
        System_abort("MAX30105::writeRegister8 Error - Bad I2C transfer!");
    }

}

uint16_t MCP9808::read16(I2C_Handle i2c, uint8_t regAddr) {
//  uint16_t val;
//
//  Wire.beginTransmission(_i2caddr);
//  Wire.write((uint8_t)reg);
//  Wire.endTransmission();
//
//  Wire.requestFrom((uint8_t)_i2caddr, (uint8_t)2);
//  val = Wire.read();
//  val <<= 8;
//  val |= Wire.read();
//  return val;

    uint8_t             txBuffer[2], rxBuffer[2];
    uint16_t            aggregateRX;


        I2C_Transaction     i2cTransaction;

        i2cTransaction.slaveAddress = Board_MCP9808_ADDR;

        // Write to an 8-bit status register
        i2cTransaction.writeBuf = txBuffer;
        i2cTransaction.readBuf = rxBuffer;
        i2cTransaction.writeCount = 1;
        i2cTransaction.readCount = 2;

        txBuffer[0] = regAddr;

        // if (!I2C_transfer(handle, &i2cTransaction)) {      this was how this was prior to making MAX30105 class. See if next line works...
        if (!I2C_transfer(i2c, &i2cTransaction)) {
            GPIO_write(Board_LED1, Board_LED_ON);
            System_abort("Bad I2C transfer!");
        } // else {
            aggregateRX = rxBuffer[0];
            aggregateRX = aggregateRX + rxBuffer[1];
            return (aggregateRX);
        // }


}




