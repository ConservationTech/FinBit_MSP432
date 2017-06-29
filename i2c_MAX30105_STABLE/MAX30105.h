/*
 * MAX30105.h
 *
 *  Created on: Jun 17, 2017
 *      Author: dave
 */

#ifndef MAX30105_H_
#define MAX30105_H_

#include <stdint.h>
// #include "MSP_EXP432P401R.h"

#define MAX30105_ADDRESS          0x57 //7-bit I2C Address
//Note that MAX30102 has the same I2C address and Part ID

#define I2C_SPEED_STANDARD        100000
#define I2C_SPEED_FAST            400000

// Extra definitions to try and get this compiling - DKH 24 June 2017

#define Board_initGeneral           MSP_EXP432P401R_initGeneral
#define Board_initGPIO              MSP_EXP432P401R_initGPIO
#define Board_initI2C               MSP_EXP432P401R_initI2C
#define Board_I2C_B0P16P17          MSP_EXP432P401R_I2CB0
#define Board_MAX30105_ADDR         (0x57)
#define Board_MPU9250_ADDR          (0x68)                  // Sparkfun MPU-9250 Breakout  - 9-DOF Accel + Mag + Gyro IMU Sensor Package
#define Board_MPL3115A2_ADDR        (0xCO)                  // Sparkfun MPL3115A2 Breakout - Altitude and Temperature (??) Sensor Package
#define Board_ADS1115_ADDR          (0x48)                  // Adafruit ADS1115 Breakout   - ADS1115 16-bit A0-A4 ADC with I2C Interface
#define Board_MCP9808_ADDR          (0x18)                  // Adafruit MCP9808 Breakout   - MCP9808 Temperature Sensor


class MAX30105 {

    public:

        MAX30105(void);

        bool begin(I2C_Handle i2c);

        uint32_t getRed(I2C_Handle i2c); //Returns immediate red value
        uint32_t getIR(I2C_Handle i2c); //Returns immediate IR value
        uint32_t getGreen(I2C_Handle i2c); //Returns immediate green value
        bool safeCheck(uint8_t maxTimeToCheck); //Given a max amount of time, check for new data

        // Configuration
        void softReset(I2C_Handle i2c);
        void shutDown(I2C_Handle i2c);
        void wakeUp(I2C_Handle i2c);

        void setLEDMode(I2C_Handle i2c, uint8_t mode);

        void setADCRange(I2C_Handle i2c, uint8_t adcRange);
        void setSampleRate(I2C_Handle i2c, uint8_t sampleRate);
        void setPulseWidth(I2C_Handle i2c, uint8_t pulseWidth);

        void setPulseAmplitudeRed(I2C_Handle i2c, uint8_t value);
        void setPulseAmplitudeIR(I2C_Handle i2c, uint8_t value);
        void setPulseAmplitudeGreen(I2C_Handle i2c, uint8_t value);
        void setPulseAmplitudeProximity(I2C_Handle i2c, uint8_t value);

        void setProximityThreshold(I2C_Handle i2c, uint8_t threshMSB);

        //Multi-led configuration mode (page 22)
        void enableSlot(I2C_Handle i2c, uint8_t slotNumber, uint8_t device); //Given slot number, assign a device to slot
        void disableSlots(I2C_Handle i2c);

        //Data Collection

        //Interrupts (page 13, 14)
        uint8_t getINT1(I2C_Handle i2c); //Returns the main interrupt group
        uint8_t getINT2(I2C_Handle i2c); //Returns the temp ready interrupt
        void enableAFULL(I2C_Handle i2c); //Enable/disable individual interrupts
        void disableAFULL(I2C_Handle i2c);
        void enableDATARDY(I2C_Handle i2c);
        void disableDATARDY(I2C_Handle i2c);
        void enableALCOVF(I2C_Handle i2c);
        void disableALCOVF(I2C_Handle i2c);
        void enablePROXINT(I2C_Handle i2c);
        void disablePROXINT(I2C_Handle i2c);
        void enableDIETEMPRDY(I2C_Handle i2c);
        void disableDIETEMPRDY(I2C_Handle i2c);

        //FIFO Configuration (page 18)
        void setFIFOAverage(I2C_Handle i2c, uint8_t samples);
        void enableFIFORollover(I2C_Handle i2c);
        void disableFIFORollover(I2C_Handle i2c);
        void setFIFOAlmostFull(I2C_Handle i2c, uint8_t samples);

        //FIFO Reading
        uint16_t check(I2C_Handle i2c); //Checks for new data and fills FIFO
        uint8_t available(void); //Tells caller how many new samples are available (head - tail)
        void nextSample(void); //Advances the tail of the sense array
        uint32_t getFIFORed(I2C_Handle i2c); //Returns the FIFO sample pointed to by tail
        uint32_t getFIFOIR(I2C_Handle i2c); //Returns the FIFO sample pointed to by tail
        uint32_t getFIFOGreen(I2C_Handle i2c); //Returns the FIFO sample pointed to by tail

        uint8_t getWritePointer(I2C_Handle i2c);
        uint8_t getReadPointer(I2C_Handle i2c);
        void clearFIFO(I2C_Handle i2c); //Sets the read/write pointers to zero

        //Proximity Mode Interrupt Threshold
        void setPROXINTTHRESH(I2C_Handle i2c, uint8_t val);

        // Die Temperature
        float readTemperature(I2C_Handle i2c);
        float readTemperatureF(I2C_Handle i2c);

        // Detecting ID/Revision
        uint8_t getRevisionID();
        uint8_t readPartID(I2C_Handle i2c);

        // Setup the IC with user selectable settings
        void setup(I2C_Handle i2c, uint8_t powerLevel = 0x1F, uint8_t sampleAverage = 4, uint8_t ledMode = 3, uint16_t sampleRate = 400, uint16_t pulseWidth = 411, uint16_t adcRange = 4096);

        uint8_t readRegister8(I2C_Handle i2c, uint8_t regAddr);
        void    writeRegister8(I2C_Handle i2c, uint8_t regAddr, uint8_t value);

    private:

        I2C_Handle *i2c;                                            // Generic connection to MSP432's I2C_Handle

        void bitMask(I2C_Handle i2c, uint8_t regAddr, uint8_t mask, uint8_t thing);

        //activeLEDs is the number of channels turned on, and can be 1 to 3. 2 is common for Red+IR.
        uint8_t activeLEDs; //Gets set during setup. Allows check() to calculate how many bytes to read from FIFO

        uint8_t revisionID;

        void readRevisionID(I2C_Handle i2c);
};

/* ======== BRING THESE UP ONE AT A TIME AS THE I2C STUFF GETS SORTED OUT









};

*/
#endif /* MAX30105_H_ */
