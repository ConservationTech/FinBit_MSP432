/*
 * MCP9808.h
 *
 *  Created on: Jul 10, 2017
 *      Author: dave
 */

/**************** MODIFIED FROM THIS ORIGINAL CODE **********************/
/*!
    @file     Adafruit_MCP9808.h
    @author   K. Townsend (Adafruit Industries)
    @license  BSD (see license.txt)

    This is a library for the Adafruit MCP9808 Temp Sensor breakout board
    ----> http://www.adafruit.com/products/1782

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0  - First release
*/
/**************************************************************************/

#ifndef MCP9808_H_
#define MCP9808_H_

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

#define I2C_SPEED_STANDARD        100000
#define I2C_SPEED_FAST            400000

// Extra definitions to try and get this compiling - DKH 24 June 2017

#define Board_initGeneral           MSP_EXP432P401R_initGeneral
#define Board_initGPIO              MSP_EXP432P401R_initGPIO
#define Board_initI2C               MSP_EXP432P401R_initI2C
#define Board_I2C_B0P16P17          MSP_EXP432P401R_I2CB0
#define Board_MAX30105_ADDR         (0x57)

#define MCP9808_I2CADDR_DEFAULT        0x18
#define MCP9808_REG_CONFIG             0x01

#define MCP9808_REG_CONFIG_SHUTDOWN    0x0100
#define MCP9808_REG_CONFIG_CRITLOCKED  0x0080
#define MCP9808_REG_CONFIG_WINLOCKED   0x0040
#define MCP9808_REG_CONFIG_INTCLR      0x0020
#define MCP9808_REG_CONFIG_ALERTSTAT   0x0010
#define MCP9808_REG_CONFIG_ALERTCTRL   0x0008
#define MCP9808_REG_CONFIG_ALERTSEL    0x0004
#define MCP9808_REG_CONFIG_ALERTPOL    0x0002
#define MCP9808_REG_CONFIG_ALERTMODE   0x0001

#define MCP9808_REG_UPPER_TEMP         0x02
#define MCP9808_REG_LOWER_TEMP         0x03
#define MCP9808_REG_CRIT_TEMP          0x04
#define MCP9808_REG_AMBIENT_TEMP       0x05
#define MCP9808_REG_MANUF_ID           0x06
#define MCP9808_REG_DEVICE_ID          0x07

class MCP9808 {
 public:
  MCP9808(void);
  bool begin(I2C_Handle i2c, uint8_t regAddr); // , uint8_t Board_MCP9808_ADDR);
  float readTempF( I2C_Handle i2c );
  float readTempC( I2C_Handle i2c );
  void shutdown_wake( I2C_Handle i2c, uint8_t sw_ID );
  void shutdown(I2C_Handle i2c);
  void wake(I2C_Handle i2c);

  void write16(I2C_Handle i2c, uint8_t regAddr, uint16_t val);
  uint16_t read16(I2C_Handle i2c, uint8_t regAddr);

 private:

  // uint8_t i2c;
  I2C_Handle *i2c;                                            // Generic connection to MSP432's I2C_Handle

};

// #endif



#endif /* MCP9808_H_ */
