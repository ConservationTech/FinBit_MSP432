/*
 * MAX30105.cpp
 *
 *  Created on: Jun 29, 2017
 *      Author: dave
 */


 /*    ======== i2c_MAX30105.c
 */

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



/* Include the MAX30105 header file */
#include "MAX30105.h"

#define DEBUGPRINT          false
#define TASKSTACKSIZE       640

// Status Registers
static const uint8_t MAX30105_INTSTAT1 =        0x00;
static const uint8_t MAX30105_INTSTAT2 =        0x01;
static const uint8_t MAX30105_INTENABLE1 =      0x02;
static const uint8_t MAX30105_INTENABLE2 =      0x03;

// FIFO Registers
static const uint8_t MAX30105_FIFOWRITEPTR =    0x04;
static const uint8_t MAX30105_FIFOOVERFLOW =    0x05;
static const uint8_t MAX30105_FIFOREADPTR =     0x06;
static const uint8_t MAX30105_FIFODATA =        0x07;

// Configuration Registers
static const uint8_t MAX30105_FIFOCONFIG =      0x08;
static const uint8_t MAX30105_MODECONFIG =      0x09;
static const uint8_t MAX30105_PARTICLECONFIG =  0x0A;    // Note, sometimes listed as "SPO2" config in datasheet (pg. 11)
static const uint8_t MAX30105_LED1_PULSEAMP =   0x0C;
static const uint8_t MAX30105_LED2_PULSEAMP =   0x0D;
static const uint8_t MAX30105_LED3_PULSEAMP =   0x0E;
static const uint8_t MAX30105_LED_PROX_AMP =    0x10;
static const uint8_t MAX30105_MULTILEDCONFIG1 = 0x11;
static const uint8_t MAX30105_MULTILEDCONFIG2 = 0x12;

// Die Temperature Registers
static const uint8_t MAX30105_DIETEMPINT =      0x1F;
static const uint8_t MAX30105_DIETEMPFRAC =     0x20;
static const uint8_t MAX30105_DIETEMPCONFIG =   0x21;

// Proximity Function Registers
static const uint8_t MAX30105_PROXINTTHRESH =   0x30;

// Part ID Registers
static const uint8_t MAX30105_REVISIONID =      0xFE;
static const uint8_t MAX30105_PARTID =          0xFF;    // Should always be 0x15. Identical to MAX30102.

// MAX30105 Commands
// Interrupt configuration (pg 13, 14)

// Bit 7 - Interrupt Status 1
/* static const uint8_t MAX30105_INT_A_FULL_MASK =     (byte)~0b10000000;
 *  >>>>> trying to force these (byte)~0bxxxxxxxx constructions into type uint8_t <<<<<
 */
static const uint8_t MAX30105_INT_A_FULL_MASK =     (uint8_t)~0b10000000;
static const uint8_t MAX30105_INT_A_FULL_ENABLE =   0x80;
static const uint8_t MAX30105_INT_A_FULL_DISABLE =  0x00;

// Bit 6 - Interrupt Status 1
static const uint8_t MAX30105_INT_DATA_RDY_MASK = (uint8_t)~0b01000000;
static const uint8_t MAX30105_INT_DATA_RDY_ENABLE = 0x40;
static const uint8_t MAX30105_INT_DATA_RDY_DISABLE = 0x00;

// Bit 5 - Interrupt Status 1
static const uint8_t MAX30105_INT_ALC_OVF_MASK = (uint8_t)~0b00100000;
static const uint8_t MAX30105_INT_ALC_OVF_ENABLE =  0x20;
static const uint8_t MAX30105_INT_ALC_OVF_DISABLE = 0x00;

// Bit 4 - Interrupt Status 1
static const uint8_t MAX30105_INT_PROX_INT_MASK = (uint8_t)~0b00010000;
static const uint8_t MAX30105_INT_PROX_INT_ENABLE = 0x10;
static const uint8_t MAX30105_INT_PROX_INT_DISABLE = 0x00;

// Bit 2 - Interrupt Status 2
static const uint8_t MAX30105_INT_DIE_TEMP_RDY_MASK = (uint8_t)~0b00000010;
static const uint8_t MAX30105_INT_DIE_TEMP_RDY_ENABLE = 0x02;
static const uint8_t MAX30105_INT_DIE_TEMP_RDY_DISABLE = 0x00;

static const uint8_t MAX30105_SAMPLEAVG_MASK =  (uint8_t)~0b11100000;
static const uint8_t MAX30105_SAMPLEAVG_1 =     0x00;
static const uint8_t MAX30105_SAMPLEAVG_2 =     0x20;
static const uint8_t MAX30105_SAMPLEAVG_4 =     0x40;
static const uint8_t MAX30105_SAMPLEAVG_8 =     0x60;
static const uint8_t MAX30105_SAMPLEAVG_16 =    0x80;
static const uint8_t MAX30105_SAMPLEAVG_32 =    0xA0;

static const uint8_t MAX30105_ROLLOVER_MASK =   0xEF;
static const uint8_t MAX30105_ROLLOVER_ENABLE = 0x10;
static const uint8_t MAX30105_ROLLOVER_DISABLE = 0x00;

static const uint8_t MAX30105_A_FULL_MASK =     0xF0;

// Mode configuration commands (page 19)
static const uint8_t MAX30105_SHUTDOWN_MASK =   0x7F;
static const uint8_t MAX30105_SHUTDOWN =        0x80;
static const uint8_t MAX30105_WAKEUP =          0x00;

static const uint8_t MAX30105_RESET_MASK =      0xBF;
static const uint8_t MAX30105_RESET =           0x40;

static const uint8_t MAX30105_MODE_MASK =       0xF8;
static const uint8_t MAX30105_MODE_REDONLY =    0x02;
static const uint8_t MAX30105_MODE_REDIRONLY =  0x03;
static const uint8_t MAX30105_MODE_MULTILED =   0x07;

// Particle sensing configuration commands (pgs 19-20)
static const uint8_t MAX30105_ADCRANGE_MASK =   0x9F;
static const uint8_t MAX30105_ADCRANGE_2048 =   0x00;
static const uint8_t MAX30105_ADCRANGE_4096 =   0x20;
static const uint8_t MAX30105_ADCRANGE_8192 =   0x40;
static const uint8_t MAX30105_ADCRANGE_16384 =  0x60;

static const uint8_t MAX30105_SAMPLERATE_MASK = 0xE3;
static const uint8_t MAX30105_SAMPLERATE_50 =   0x00;
static const uint8_t MAX30105_SAMPLERATE_100 =  0x04;
static const uint8_t MAX30105_SAMPLERATE_200 =  0x08;
static const uint8_t MAX30105_SAMPLERATE_400 =  0x0C;
static const uint8_t MAX30105_SAMPLERATE_800 =  0x10;
static const uint8_t MAX30105_SAMPLERATE_1000 = 0x14;
static const uint8_t MAX30105_SAMPLERATE_1600 = 0x18;
static const uint8_t MAX30105_SAMPLERATE_3200 = 0x1C;

static const uint8_t MAX30105_PULSEWIDTH_MASK = 0xFC;
static const uint8_t MAX30105_PULSEWIDTH_69 =   0x00;
static const uint8_t MAX30105_PULSEWIDTH_118 =  0x01;
static const uint8_t MAX30105_PULSEWIDTH_215 =  0x02;
static const uint8_t MAX30105_PULSEWIDTH_411 =  0x03;

//Multi-LED Mode configuration (pg 22)
static const uint8_t MAX30105_SLOT1_MASK =      0xF8;
static const uint8_t MAX30105_SLOT2_MASK =      0x8F;
static const uint8_t MAX30105_SLOT3_MASK =      0xF8;
static const uint8_t MAX30105_SLOT4_MASK =      0x8F;

static const uint8_t SLOT_NONE =                0x00;
static const uint8_t SLOT_RED_LED =             0x01;
static const uint8_t SLOT_IR_LED =              0x02;
static const uint8_t SLOT_GREEN_LED =           0x03;
static const uint8_t SLOT_NONE_PILOT =          0x04;
static const uint8_t SLOT_RED_PILOT =           0x05;
static const uint8_t SLOT_IR_PILOT =            0x06;
static const uint8_t SLOT_GREEN_PILOT =         0x07;

static const uint8_t MAX_30105_EXPECTEDPARTID = 0x15;

//The MAX30105 stores up to 32 samples on the IC
//This is additional local storage to the microcontroller
const int STORAGE_SIZE = 4; //Each long is 4 bytes so limit this to fit on your micro

struct Record
{
  //uint32_t red[STORAGE_SIZE];
  uint32_t red[STORAGE_SIZE];
  uint32_t nir[STORAGE_SIZE];
  uint32_t green[STORAGE_SIZE];
  uint8_t head;
  uint8_t tail;
} sense;                            // This is our circular buffer of readings from the sensor

/* =================== MAX30105 CONSTRUCTOR MEMBER DEFINITIONS =================== */

MAX30105::MAX30105() {              // MAX30105 Class Constructor
  // Constructor
}

// bool MAX30105::begin(I2C_Handle i2c, uint32_t i2cSpeed, uint8_t i2caddr) {
bool MAX30105::begin(I2C_Handle i2c) {

//    I2C_Handle      i2c;
//    I2C_Params      i2cParams;
//    I2C_Params_init(&i2cParams);
//
////    if (i2cSpeed == 100000) {
////        //_i2cPort->setClock(i2cSpeed);
////        i2cParams.bitRate = I2C_100kHz;     // Set i2c bus speed to 100 kHz
////    } else if (i2cSpeed == 400000) {
////        //_i2cPort->setClock(i2cSpeed);
////        i2cParams.bitRate = I2C_400kHz;     // Set i2c bus speed to 400 kHz
////    } else {
////        // Catch any conditions where this gets specified poorly by the user's invocation, defaulting to 100 kHz
////        i2cParams.bitRate = I2C_100kHz;     // Default to 100kHz if argument is invalid bus speed
////    }
//
//    i2cParams.bitRate = I2C_400kHz;     // Set i2c bus speed to 400 kHz
//    i2c = I2C_open(Board_I2C_B0P16P17, &i2cParams);   // Board_I2C_MAX30105 -> MSP432's Pins 1.7 and 1.6 by default = UCB0SDA and UCB0SCL
//                                                      // ... may want to build in a spec for UCB1SDA and UCB1SCL to Pins 6.5 and 6.4
//                                                      // ... see MSP_EXP432P401R.h around Line 91 to do this enum

    if (i2c == NULL) {
        System_abort("Error Initializing I2C inside MAX30105 class. That's a problem!\n");
        System_flush();
        return false;
    }
    else {
        if (!readPartID(i2c) == MAX_30105_EXPECTEDPARTID) {
          // Error -- Part ID read from MAX30105 does not match expected part ID.
          // This may mean there is a physical connectivity problem (broken wire, unpowered, etc).
          return false;
    }

        if (DEBUGPRINT) {
            System_printf("Opening communication with MAX30105... time to gather some data!\n");
            System_flush();
        }

    return true;

    }

}

void MAX30105::softReset(I2C_Handle i2c) {
  bitMask(i2c, MAX30105_MODECONFIG, MAX30105_RESET_MASK, MAX30105_RESET);

  // Poll for bit to clear, reset is then complete; give timeout of ~100ms

  Task_sleep(100);
  uint8_t response = readRegister8(i2c, MAX30105_MODECONFIG);
  if ((response & MAX30105_RESET) == 0) {
      if (DEBUGPRINT) {
          System_printf("MAX30105 soft reset: completion successful!\n");
          System_flush();
      }
  }
  Task_sleep(10);     // delay(1); //Let's not over burden the I2C bus

}

void MAX30105::shutDown(I2C_Handle i2c) {
  // Put IC into low power mode (datasheet pg. 19)
  // During shutdown the IC will continue to respond to I2C commands but will
  // not update with or take new readings (such as temperature)
  bitMask(i2c, MAX30105_MODECONFIG, MAX30105_SHUTDOWN_MASK, MAX30105_SHUTDOWN);
}

void MAX30105::wakeUp(I2C_Handle i2c) {
  // Pull IC out of low power mode (datasheet pg. 19)
  bitMask(i2c, MAX30105_MODECONFIG, MAX30105_SHUTDOWN_MASK, MAX30105_WAKEUP);
}

void MAX30105::setLEDMode(I2C_Handle i2c, uint8_t mode) {
  // Set which LEDs are used for sampling -- Red only, RED+IR only, or custom.
  // See datasheet, page 19
  bitMask(i2c, MAX30105_MODECONFIG, MAX30105_MODE_MASK, mode);
}

void MAX30105::setADCRange(I2C_Handle i2c, uint8_t adcRange) {
  // adcRange: one of MAX30105_ADCRANGE_2048, _4096, _8192, _16384
  bitMask(i2c, MAX30105_PARTICLECONFIG, MAX30105_ADCRANGE_MASK, adcRange);
}

void MAX30105::setSampleRate(I2C_Handle i2c, uint8_t sampleRate) {
  // sampleRate: one of MAX30105_SAMPLERATE_50, _100, _200, _400, _800, _1000, _1600, _3200
  bitMask(i2c, MAX30105_PARTICLECONFIG, MAX30105_SAMPLERATE_MASK, sampleRate);
}

void MAX30105::setPulseWidth(I2C_Handle i2c, uint8_t pulseWidth) {
  // pulseWidth: one of MAX30105_PULSEWIDTH_69, _188, _215, _411
  bitMask(i2c, MAX30105_PARTICLECONFIG, MAX30105_PULSEWIDTH_MASK, pulseWidth);
}

// NOTE: Amplitude values: 0x00 = 0mA, 0x7F = 25.4mA, 0xFF = 50mA (typical)
// See datasheet, page 21
void MAX30105::setPulseAmplitudeRed(I2C_Handle i2c, uint8_t amplitude) {
  writeRegister8(i2c, MAX30105_LED1_PULSEAMP, amplitude);
}

void MAX30105::setPulseAmplitudeIR(I2C_Handle i2c, uint8_t amplitude) {
  writeRegister8(i2c, MAX30105_LED2_PULSEAMP, amplitude);
}

void MAX30105::setPulseAmplitudeGreen(I2C_Handle i2c, uint8_t amplitude) {
  writeRegister8(i2c, MAX30105_LED3_PULSEAMP, amplitude);
}

void MAX30105::setPulseAmplitudeProximity(I2C_Handle i2c, uint8_t amplitude) {
  writeRegister8(i2c, MAX30105_LED_PROX_AMP, amplitude);
}

void MAX30105::setProximityThreshold(I2C_Handle i2c, uint8_t threshMSB) {
  // Set the IR ADC count that will trigger the beginning of particle-sensing mode.
  // The threshMSB signifies only the 8 most significant-bits of the ADC count.
  // See datasheet, page 24.
  writeRegister8(i2c, MAX30105_PROXINTTHRESH, threshMSB);
}

//Given a slot number assign a thing to it
//Devices are SLOT_RED_LED or SLOT_RED_PILOT (proximity)
//Assigning a SLOT_RED_LED will pulse LED
//Assigning a SLOT_RED_PILOT will ??
void MAX30105::enableSlot(I2C_Handle i2c, uint8_t slotNumber, uint8_t device) {

  // uint8_t originalContents;    // Think this is vestigial. If commenting doesn't blow up everything, remove this line. -DKH

  switch (slotNumber) {
    case (1):
      bitMask(i2c, MAX30105_MULTILEDCONFIG1, MAX30105_SLOT1_MASK, device);
      break;
    case (2):
      bitMask(i2c, MAX30105_MULTILEDCONFIG1, MAX30105_SLOT2_MASK, device << 4);
      break;
    case (3):
      bitMask(i2c, MAX30105_MULTILEDCONFIG2, MAX30105_SLOT3_MASK, device);
      break;
    case (4):
      bitMask(i2c, MAX30105_MULTILEDCONFIG2, MAX30105_SLOT4_MASK, device << 4);
      break;
    default:
      //Shouldn't be here!
      break;
  }
}

//Clears all slot assignments
void MAX30105::disableSlots(I2C_Handle i2c) {
  writeRegister8(i2c, MAX30105_MULTILEDCONFIG1, 0);
  writeRegister8(i2c, MAX30105_MULTILEDCONFIG2, 0);
}

//
// FIFO Configuration
//

//Set sample average (Table 3, Page 18)
void MAX30105::setFIFOAverage(I2C_Handle i2c, uint8_t numberOfSamples) {
  bitMask(i2c, MAX30105_FIFOCONFIG, MAX30105_SAMPLEAVG_MASK, numberOfSamples);
}

//Resets all points to start in a known state
//Page 15 recommends clearing FIFO before beginning a read
void MAX30105::clearFIFO(I2C_Handle i2c) {
  writeRegister8(i2c, MAX30105_FIFOWRITEPTR, 0);
  writeRegister8(i2c, MAX30105_FIFOOVERFLOW, 0);
  writeRegister8(i2c, MAX30105_FIFOREADPTR, 0);
}

//Enable roll over if FIFO over flows
void MAX30105::enableFIFORollover(I2C_Handle i2c) {
  bitMask(i2c, MAX30105_FIFOCONFIG, MAX30105_ROLLOVER_MASK, MAX30105_ROLLOVER_ENABLE);
}

//Disable roll over if FIFO over flows
void MAX30105::disableFIFORollover(I2C_Handle i2c) {
  bitMask(i2c, MAX30105_FIFOCONFIG, MAX30105_ROLLOVER_MASK, MAX30105_ROLLOVER_DISABLE);
}

//Set number of samples to trigger the almost full interrupt (Page 18)
//Power on default is 32 samples
//Note it is reverse: 0x00 is 32 samples, 0x0F is 17 samples
void MAX30105::setFIFOAlmostFull(I2C_Handle i2c, uint8_t numberOfSamples) {
  bitMask(i2c, MAX30105_FIFOCONFIG, MAX30105_A_FULL_MASK, numberOfSamples);
}

//Read the FIFO Write Pointer
uint8_t MAX30105::getWritePointer(I2C_Handle i2c) {
  return (readRegister8(i2c, MAX30105_FIFOWRITEPTR));
}

//Read the FIFO Read Pointer
uint8_t MAX30105::getReadPointer(I2C_Handle i2c) {
  return (readRegister8(i2c, MAX30105_FIFOREADPTR));
}

// Set the PROX_INT_THRESHold
void MAX30105::setPROXINTTHRESH(I2C_Handle i2c, uint8_t val) {
  writeRegister8(i2c, MAX30105_PROXINTTHRESH, val);
}


// readPartID from MAX30105

uint8_t MAX30105::readPartID(I2C_Handle i2c) {
    return (readRegister8(i2c, MAX30105_PARTID));
}

void MAX30105::readRevisionID(I2C_Handle i2c) {
  revisionID = readRegister8(i2c, MAX30105_REVISIONID);
}

uint8_t MAX30105::getRevisionID() {
  return revisionID;
}

//
// Configuration
//


//Begin Interrupt configuration
uint8_t MAX30105::getINT1(I2C_Handle i2c) {
  return (readRegister8(i2c, MAX30105_INTSTAT1));
}
uint8_t MAX30105::getINT2(I2C_Handle i2c) {
  return (readRegister8(i2c, MAX30105_INTSTAT2));
}

void MAX30105::enableAFULL(I2C_Handle i2c) {
  bitMask(i2c, MAX30105_INTENABLE1, MAX30105_INT_A_FULL_MASK, MAX30105_INT_A_FULL_ENABLE);
}
void MAX30105::disableAFULL(I2C_Handle i2c) {
  bitMask(i2c, MAX30105_INTENABLE1, MAX30105_INT_A_FULL_MASK, MAX30105_INT_A_FULL_DISABLE);
}

void MAX30105::enableDATARDY(I2C_Handle i2c) {
  bitMask(i2c, MAX30105_INTENABLE1, MAX30105_INT_DATA_RDY_MASK, MAX30105_INT_DATA_RDY_ENABLE);
}
void MAX30105::disableDATARDY(I2C_Handle i2c) {
  bitMask(i2c, MAX30105_INTENABLE1, MAX30105_INT_DATA_RDY_MASK, MAX30105_INT_DATA_RDY_DISABLE);
}

void MAX30105::enableALCOVF(I2C_Handle i2c) {
  bitMask(i2c, MAX30105_INTENABLE1, MAX30105_INT_ALC_OVF_MASK, MAX30105_INT_ALC_OVF_ENABLE);
}
void MAX30105::disableALCOVF(I2C_Handle i2c) {
  bitMask(i2c, MAX30105_INTENABLE1, MAX30105_INT_ALC_OVF_MASK, MAX30105_INT_ALC_OVF_DISABLE);
}

void MAX30105::enablePROXINT(I2C_Handle i2c) {
  bitMask(i2c, MAX30105_INTENABLE1, MAX30105_INT_PROX_INT_MASK, MAX30105_INT_PROX_INT_ENABLE);
}
void MAX30105::disablePROXINT(I2C_Handle i2c) {
  bitMask(i2c, MAX30105_INTENABLE1, MAX30105_INT_PROX_INT_MASK, MAX30105_INT_PROX_INT_DISABLE);
}

void MAX30105::enableDIETEMPRDY(I2C_Handle i2c) {
  bitMask(i2c, MAX30105_INTENABLE2, MAX30105_INT_DIE_TEMP_RDY_MASK, MAX30105_INT_DIE_TEMP_RDY_ENABLE);
}
void MAX30105::disableDIETEMPRDY(I2C_Handle i2c) {
  bitMask(i2c, MAX30105_INTENABLE2, MAX30105_INT_DIE_TEMP_RDY_MASK, MAX30105_INT_DIE_TEMP_RDY_DISABLE);
}

// Setup the sensor. The MAX30105 has many settings. By default we select:
//          Sample Average = 4
//          Mode = MultiLED
//          ADC Range = 16384 (62.5pA per LSB)
//          Sample rate = 50

void MAX30105::setup(I2C_Handle i2c, uint8_t powerLevel, uint8_t sampleAverage, uint8_t ledMode, uint16_t sampleRate, uint16_t pulseWidth, uint16_t adcRange) {
  softReset(i2c); //Reset all configuration, threshold, and data registers to POR values

  //FIFO Configuration
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //The chip will average multiple samples of same type together if you wish
  if (sampleAverage == 1) setFIFOAverage(i2c, MAX30105_SAMPLEAVG_1); //No averaging per FIFO record
  else if (sampleAverage == 2) setFIFOAverage(i2c, MAX30105_SAMPLEAVG_2);
  else if (sampleAverage == 4) setFIFOAverage(i2c, MAX30105_SAMPLEAVG_4);
  else if (sampleAverage == 8) setFIFOAverage(i2c, MAX30105_SAMPLEAVG_8);
  else if (sampleAverage == 16) setFIFOAverage(i2c, MAX30105_SAMPLEAVG_16);
  else if (sampleAverage == 32) setFIFOAverage(i2c, MAX30105_SAMPLEAVG_32);
  else setFIFOAverage(i2c, MAX30105_SAMPLEAVG_4);

  //setFIFOAlmostFull(2); //Set to 30 samples to trigger an 'Almost Full' interrupt
  enableFIFORollover(i2c); //Allow FIFO to wrap/roll over
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  //Mode Configuration
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  if (ledMode == 3) setLEDMode(i2c, MAX30105_MODE_MULTILED); //Watch all three LED channels
  else if (ledMode == 2) setLEDMode(i2c, MAX30105_MODE_REDIRONLY); //Red and IR
  else setLEDMode(i2c, MAX30105_MODE_REDONLY); //Red only
  activeLEDs = ledMode; //Used to control how many bytes to read from FIFO buffer
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  //Particle Sensing Configuration
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  if(adcRange < 4096) setADCRange(i2c, MAX30105_ADCRANGE_2048); //7.81pA per LSB
  else if(adcRange < 8192) setADCRange(i2c, MAX30105_ADCRANGE_4096); //15.63pA per LSB
  else if(adcRange < 16384) setADCRange(i2c, MAX30105_ADCRANGE_8192); //31.25pA per LSB
  else if(adcRange == 16384) setADCRange(i2c, MAX30105_ADCRANGE_16384); //62.5pA per LSB
  else setADCRange(i2c, MAX30105_ADCRANGE_2048);

  if (sampleRate < 100) setSampleRate(i2c, MAX30105_SAMPLERATE_50); //Take 50 samples per second
  else if (sampleRate < 200) setSampleRate(i2c, MAX30105_SAMPLERATE_100);
  else if (sampleRate < 400) setSampleRate(i2c, MAX30105_SAMPLERATE_200);
  else if (sampleRate < 800) setSampleRate(i2c, MAX30105_SAMPLERATE_400);
  else if (sampleRate < 1000) setSampleRate(i2c, MAX30105_SAMPLERATE_800);
  else if (sampleRate < 1600) setSampleRate(i2c, MAX30105_SAMPLERATE_1000);
  else if (sampleRate < 3200) setSampleRate(i2c, MAX30105_SAMPLERATE_1600);
  else if (sampleRate == 3200) setSampleRate(i2c, MAX30105_SAMPLERATE_3200);
  else setSampleRate(i2c, MAX30105_SAMPLERATE_50);

  //The longer the pulse width the longer range of detection you'll have
  //At 69us and 0.4mA it's about 2 inches
  //At 411us and 0.4mA it's about 6 inches
  if (pulseWidth < 118) setPulseWidth(i2c, MAX30105_PULSEWIDTH_69); //Page 26, Gets us 15 bit resolution
  else if (pulseWidth < 215) setPulseWidth(i2c, MAX30105_PULSEWIDTH_118); //16 bit resolution
  else if (pulseWidth < 411) setPulseWidth(i2c, MAX30105_PULSEWIDTH_215); //17 bit resolution
  else if (pulseWidth == 411) setPulseWidth(i2c, MAX30105_PULSEWIDTH_411); //18 bit resolution
  else setPulseWidth(i2c, MAX30105_PULSEWIDTH_69);
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  //LED Pulse Amplitude Configuration
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  //Default is 0x1F which gets us 6.4mA
  //powerLevel = 0x02, 0.4mA - Presence detection of ~4 inch
  //powerLevel = 0x1F, 6.4mA - Presence detection of ~8 inch
  //powerLevel = 0x7F, 25.4mA - Presence detection of ~8 inch
  //powerLevel = 0xFF, 50.0mA - Presence detection of ~12 inch

  setPulseAmplitudeRed(i2c, powerLevel);
  setPulseAmplitudeIR(i2c, powerLevel);
  setPulseAmplitudeGreen(i2c, powerLevel);
  setPulseAmplitudeProximity(i2c, powerLevel);
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  //Multi-LED Mode Configuration, Enable the reading of the three LEDs
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
  enableSlot(i2c, 1, SLOT_RED_LED);
  if (ledMode > 1) enableSlot(i2c, 2, SLOT_IR_LED);
  if (ledMode > 2) enableSlot(i2c, 3, SLOT_GREEN_LED);
  //enableSlot(i2c, 1, SLOT_RED_PILOT);
  //enableSlot(i2c, 2, SLOT_IR_PILOT);
  //enableSlot(i2c, 3, SLOT_GREEN_PILOT);
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  clearFIFO(i2c); //Reset the FIFO before we begin checking the sensor

}

//
// Data Collection
//

//Check for new data but give up after a certain amount of time
//Returns true if new data was found
//Returns false if new data was not found

bool MAX30105::safeCheck(I2C_Handle i2c, uint8_t maxTimeToCheck)    /* Highly modified for the MSP432. Refer to original MAX30105.cpp file to see how this worked with Arduino */
{

    uint16_t numSamples;

    numSamples = check(i2c);

    if(numSamples >= 1)  {                  // We found new data!
        if (DEBUGPRINT) {
            System_printf("safeCheck() says numSamples in FIFO = %d -- via check() routine!\n", numSamples);
            System_flush();
        }
        return(true);
    } else {
        if (DEBUGPRINT) {
            System_printf("No new data reported by safeCheck... if data is in FIFO, this shouldn't happen!\n");
            System_flush();
        }
        return(false);
    }
}

//Tell caller how many samples are available
uint8_t MAX30105::available(void)
{
  uint8_t numberOfSamples = sense.head - sense.tail;

  if (numberOfSamples < 0) numberOfSamples += STORAGE_SIZE;

  return (numberOfSamples);
}

//Report the most recent red value
uint32_t MAX30105::getRed(I2C_Handle i2c)
{
  //Check for red LED data
  if(safeCheck(i2c, 100))
    return (sense.red[sense.head]);
  else
    return(0); //Sensor failed to find new data
}

//Report the most recent IR value
uint32_t MAX30105::getIR(I2C_Handle i2c)
{
  //Check for NIR LED data
  if(safeCheck(i2c, 100))
    return (sense.nir[sense.head]);
  else
    return(0); //Sensor failed to find new data
}

//Report the most recent Green value
uint32_t MAX30105::getGreen(I2C_Handle i2c)
{
  // Check for green LED data
  if(safeCheck(i2c, 100))
    return (sense.green[sense.head]);
  else
    return(0); //Sensor failed to find new data
}

//Report the next Red value in the FIFO
uint32_t MAX30105::getFIFORed(I2C_Handle i2c)
{
  return (sense.red[sense.tail]);
}

//Report the next IR value in the FIFO
uint32_t MAX30105::getFIFOIR(I2C_Handle i2c)
{
  return (sense.nir[sense.tail]);
}

//Report the next Green value in the FIFO
uint32_t MAX30105::getFIFOGreen(I2C_Handle i2c)
{
  return (sense.green[sense.tail]);
}

//Advance the tail
void MAX30105::nextSample(void)
{
  if(available()) //Only advance the tail if new data is available
  {
    sense.tail++;
    sense.tail %= STORAGE_SIZE; //Wrap condition
  }
}

// Polls the sensor for new data. Call regularly. If new data is available, this function
// updates the head and tail in the main structure, returning number of new samples obtained.

uint16_t MAX30105::check(I2C_Handle i2c)
{
  //Read register FIDO_DATA in (3-byte * number of active LED) chunks
  //Until FIFO_RD_PTR = FIFO_WR_PTR

    // uint32_t tempLong[3];
    //    tempLong[0] = 0;
    //    tempLong[1] = 1;
    //    tempLong[2] = 2;

    // Try this using a uint32_t pointer to tempLong
    uint32_t* tempLong[3];

    uint8_t readPointer = getReadPointer(i2c);
    uint8_t writePointer = getWritePointer(i2c);

    uint8_t numberOfSamples = 0;

    if (DEBUGPRINT) {
        System_printf("MAX30105 readPointer = %d | MAX30105 writePointer = %d\n", readPointer, writePointer);
        System_flush();
    }

    //Do we have new data?
    if (readPointer != writePointer)
    {
    //Calculate the number of readings we need to get from sensor
    numberOfSamples = writePointer - readPointer;

    if (DEBUGPRINT) {
        System_printf("check() reports numberOfSamples in FIFO = %d\n", numberOfSamples);
        System_flush();
    }

    if (numberOfSamples < 0) numberOfSamples += 32; //Wrap condition

    //We now have the number of readings, now calc bytes to read
    //For this example we are just doing Red and IR (3 bytes each)

    uint16_t bytesLeftToRead = numberOfSamples * activeLEDs * 3;

    if (DEBUGPRINT) {
        System_printf("check() reports bytesLeftToRead = %d\n", bytesLeftToRead);
        System_flush();
    }

    //Wire.requestFrom() is limited to BUFFER_LENGTH which is 32 on the Uno
    //We may need to read as many as 288 bytes so we read in blocks no larger than 32
    //BUFFER_LENGTH should work with other platforms with larger requestFrom buffers
    //For now, use this placeholder value of 32. MSP432 may be able to handle much more than Arduino Uno...
    uint8_t BUFFER_LENGTH = 32;

    while (bytesLeftToRead > 0)
        {
          uint16_t toGet = bytesLeftToRead;
          if (toGet > BUFFER_LENGTH)
          {
            //If toGet is 32 this is bad because we read 6 bytes (Red+IR * 3 = 6) at a time
            //32 % 6 = 2 left over. We don't want to request 32 bytes, we want to request 30.
            //32 % 9 (Red+IR+GREEN) = 5 left over. We want to request 27.

            toGet = BUFFER_LENGTH - (BUFFER_LENGTH % (activeLEDs * 3)); //Trim toGet to be a multiple of the samples we need to read
          }

          bytesLeftToRead -= toGet;

          //Request toGet number of bytes from sensor

          while (toGet > 0)
          {
            sense.head++;                   //Advance the head of the storage struct
            sense.head %= STORAGE_SIZE;     //Wrap condition

            // Need to burst read 3 bytes for each of the active LEDs
            // We call 'readFIFODATA(i2c, MAX30105_FIFODATA, activeLEDs * 3);' to accomplish this.
            // This function pushes each 18-bit red, nir, and/or green value from MAX3010's FIFODATA circular buffer
            // into the appropriate sense.LEDCOLOR[sense.head] (i.e.: the top of our sense circular buffer)
            readFIFODATA(i2c, MAX30105_FIFODATA, activeLEDs * 3);       // Return the 18-bit RED value from MAX3010's FIFODATA circular buffer to sense.red in readFIFODATA

//            if (activeLEDs > 1)
//            {
//                readFIFODATA(i2c, MAX30105_FIFODATA, activeLEDs * 3);       // Return the 18-bit RED value from MAX3010's FIFODATA circular buffer to sense.red in readFIFODATA
//
//            }
//
//            if (activeLEDs > 2)
//            {
//                // Need to parse tempLong[1:3] and stuff into uint32_t sense.red, sense.nir, and sense.green
//                readFIFODATA(i2c, MAX30105_FIFODATA, activeLEDs * 3);       // Return the 18-bit RED value from MAX3010's FIFODATA circular buffer to sense.red in readFIFODATA
//            }

            toGet -= activeLEDs * 3;
          }

        } //End while (bytesLeftToRead > 0)

    } //End readPtr != writePtr

    return (numberOfSamples); //Let the world know how much new data we found
}


float MAX30105::readTemperature(I2C_Handle i2c) {
    // Tell MAX30105 to prep tempInt and tempFrac for a read
    writeRegister8(i2c, MAX30105_DIETEMPCONFIG, 0x01);

    // Ask MAX301050 for the tempInt and tempFrac values
    int8_t tempInt = readRegister8(i2c, MAX30105_DIETEMPINT);
    uint8_t tempFrac = readRegister8(i2c, MAX30105_DIETEMPFRAC);

    // Reconstruct tempInt and tempFrac to build actual temp
    return( (float)(tempInt) ) + ( ( (float)(tempFrac) )  * 0.0625 );
}

float MAX30105::readTemperatureF(I2C_Handle i2c){
    float temp = readTemperature(i2c);
    if (temp != -999.0) temp = temp * 1.8 + 32.0;
    return (temp);
}

// readRegister8 for UNSIGNED 8-bit status registers (e.g.: MAX30105 tempFrac value)

uint8_t MAX30105::readRegister8(I2C_Handle i2c, uint8_t regAddr) {

    uint8_t             txBuffer[2], rxBuffer[1];

//    I2C_Handle      i2c;
//    I2C_Params      i2cParams;
//    I2C_Params_init(&i2cParams);                         // Create I2C for usage

    I2C_Transaction     i2cTransaction;

    i2cTransaction.slaveAddress = Board_MAX30105_ADDR;

    // Write to an 8-bit status register
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.readBuf = rxBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readCount = 1;

    txBuffer[0] = regAddr;

    // if (!I2C_transfer(handle, &i2cTransaction)) {      this was how this was prior to making MAX30105 class. See if next line works...
    if (!I2C_transfer(i2c, &i2cTransaction)) {
        GPIO_write(Board_LED1, Board_LED_ON);
        System_abort("Bad I2C transfer!");
    } // else {
        return (rxBuffer[0]);
    // }

}

void MAX30105::writeRegister8(I2C_Handle i2c, uint8_t regAddr, uint8_t value)
{
    uint8_t             txBuffer[2];
    I2C_Transaction     i2cTransaction;

    i2cTransaction.slaveAddress = Board_MAX30105_ADDR;

    // Write to an 8-bit status register
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 2;
    i2cTransaction.readCount = 0;

    txBuffer[0] = regAddr; //  & 0xFF; //LB Addr
    txBuffer[1] = value;   //   & 0xFF;

    if (!I2C_transfer(i2c, &i2cTransaction)) {
        GPIO_write(Board_LED1, Board_LED_ON);
        System_abort("MAX30105::writeRegister8 Error - Bad I2C transfer!");
    }
}

void MAX30105::readFIFODATA(I2C_Handle i2c, uint8_t regAddr, uint8_t numBytesToRead)
{

    uint32_t            ledValues[3] = {0,0,0};     // Three LEDs possible, each consisting of a 3-byte read, so divide by 3 for return values
    uint8_t             txBuffer[1], zero = 0;       // Red, NIR, and Green

    // The third declaration of uint8_t rxBuffer[3] works great for reading Red only. Now find a way to read Red, NIR, and Green programmatically...
    uint8_t             rxBuffer3[3], rxBuffer6[6], rxBuffer9[9];
    // uint8_t             rxBuffer[numBytesToRead];       // Doesn't work, as alloc throws fits over variable rxBuffer matrix variable sizing on numBytesToRead
    // uint8_t             rxBuffer[3];

    I2C_Transaction     i2cTransaction;

//    ledValues[0] = 0;
//    ledValues[1] = 0;
//    ledValues[2] = 0;
//    zero = 0;

    i2cTransaction.slaveAddress = Board_MAX30105_ADDR;

    // Write to an 8-bit status register
    i2cTransaction.writeBuf = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readCount = numBytesToRead;

    // i2cTransaction.readBuf = rxBuffer;
    // Try programmatically assigning rxBuffer depending on numBytesToRead
    if (numBytesToRead == 3) {
        i2cTransaction.readBuf = rxBuffer3;
    } else if (numBytesToRead == 6) {
        i2cTransaction.readBuf = rxBuffer6;
    } else if (numBytesToRead == 9) {
        i2cTransaction.readBuf = rxBuffer9;
    } else {
        System_printf("This condition should never happen. Panic if it does!\n");
        System_flush();
    }

    txBuffer[0] = regAddr;
    // txBuffer[1] = 0x01;

    // if (!I2C_transfer(handle, &i2cTransaction)) {      this was how this was prior to making MAX30105 class. See if next line works...
    if (!I2C_transfer(i2c, &i2cTransaction)) {
        GPIO_write(Board_LED1, Board_LED_ON);
        System_abort("Bad I2C transfer!");
    } else {

        if (numBytesToRead == 3) {
            // Read red from FIFO, pad 4th MSByte with 0x00 value
            // Led value should become a 32-bit structured as 0x00_rxBuffer[2]_rxBuffer[1])_rxBuffer[0]
            // ledValues[0] << ( ( 0x00 ) | (rxBuffer[0] << 8) | (rxBuffer[1] << 16) | (rxBuffer[2] << 24) );

            ledValues[0] = zero;
            ledValues[0] = (ledValues[0] << 8) + rxBuffer3[0];
            ledValues[0] = (ledValues[0] << 8) + rxBuffer3[1];
            ledValues[0] = (ledValues[0] << 8) + rxBuffer3[2];

            sense.red[sense.head] = ledValues[0];

            // System_printf("FIFOREAD sub-section - rxBuffer[0] [1] [2] = %d %d %d\n", rxBuffer[0], rxBuffer[1], rxBuffer[2]);
            // System_printf("FIFOREAD sub-section - ledValue[0] = %d\n", ledValues[0]);
            // System_flush();

        } else if (numBytesToRead == 6) {

            ledValues[0] = zero;
            ledValues[0] = (ledValues[0] << 8) + rxBuffer6[0];
            ledValues[0] = (ledValues[0] << 8) + rxBuffer6[1];
            ledValues[0] = (ledValues[0] << 8) + rxBuffer6[2];

            ledValues[1] = zero;
            ledValues[1] = (ledValues[1] << 8) + rxBuffer6[3];
            ledValues[1] = (ledValues[1] << 8) + rxBuffer6[4];
            ledValues[1] = (ledValues[1] << 8) + rxBuffer6[5];

            sense.red[sense.head] = ledValues[0];
            sense.nir[sense.head] = ledValues[1];

        } else if (numBytesToRead == 9) {

            ledValues[0] = zero;
            ledValues[0] = (ledValues[0] << 8) + rxBuffer9[0];
            ledValues[0] = (ledValues[0] << 8) + rxBuffer9[1];
            ledValues[0] = (ledValues[0] << 8) + rxBuffer9[2];

            ledValues[1] = zero;
            ledValues[1] = (ledValues[1] << 8) + rxBuffer9[3];
            ledValues[1] = (ledValues[1] << 8) + rxBuffer9[4];
            ledValues[1] = (ledValues[1] << 8) + rxBuffer9[5];

            ledValues[2] = zero;
            ledValues[2] = (ledValues[2] << 8) + rxBuffer9[6];
            ledValues[2] = (ledValues[2] << 8) + rxBuffer9[7];
            ledValues[2] = (ledValues[2] << 8) + rxBuffer9[8];

            sense.red[sense.head] = ledValues[0];
            sense.nir[sense.head] = ledValues[1];
            sense.green[sense.head] = ledValues[1];

        } else {
            System_printf("This is a weird condition, where numBytesToRead != 3, 6 or 9. Ruh roh.\n");
            System_flush();
        }

        // return(ledValues);
    }

    // return(*ledValues);

}

void MAX30105::bitMask(I2C_Handle i2c, uint8_t regAddr, uint8_t mask, uint8_t thing)                            //Given a register, read it, mask it, and then set the thing
{
  // Grab current register context
  uint8_t originalContents = readRegister8(i2c, regAddr);

  // Zero-out the portions of the register we're interested in
  originalContents = originalContents & mask;

  // Change contents
  writeRegister8(i2c, regAddr, originalContents | thing);
}



