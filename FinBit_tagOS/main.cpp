 /* 
 *    MAX30105 over i2c by Dave Haas - created on 17 June 2017
 *    Based an i2c example from TI for MSP432 LaunchPad.
 *    Code to address the MAX30105 module was copied from
 *    MAX30105.h and MAX30105.cpp by Peter Jansen and Nathan Seidle (SparkFun). 
 *    This code and all code for MAX30105 are governed under the
 *    BSD license, so all text above must be included in any redistribution.
 */

 /*    ======== i2c_MAX30105.c
 */

/* XDCtools Header files */
#include <xdc/runtime/System.h>
#include <xdc/std.h>
#include <xdc/runtime/Diags.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>

/* Include the generic I2C helper function prototypes */
#include "i2c_helper.h"

/* Include system_clock.h for clock and time functions */
#include "system_clock.h"

/* Example/Board Header files */
#include "Board.h"

/* Include the MAX30105 header file */
#include "MAX30105.h"

/* Include the MAX30105 header file */
#include "MCP9808.h"

#define TASKSTACKSIZE       640
#define DEBUGPRINT          true           // DEBUGPRINT = false

// static const bool DEBUGPRINT = false;          // not working the way I want it to work...

void clockIdle(void);

Task_Struct task0Struct, task1Struct, task2Struct;          // Added task1Struct and task2Struct after bigtime.cpp integration
Char task0Stack[TASKSTACKSIZE], task1Stack[TASKSTACKSIZE], task2Stack[TASKSTACKSIZE];
Semaphore_Struct sem0Struct, sem1Struct;
Semaphore_Handle sem0Handle, sem1Handle;
Clock_Struct clk0Struct, clk1Struct;

// Task_Struct task0Struct, task1Struct;                    // Commented out, since added above
// Char task0Stack[TASKSTACKSIZE], task1Stack[TASKSTACKSIZE];  // Commented out, since added above

///* Global clock objects */
Clock cl0(0);  /* idle loop clock */
Clock cl1(1);  /* periodic clock, period = 1 ms */
Clock cl2(2);  /* periodic clock, period = 1 sec */
Clock cl3(3);  /* task clock */
Clock cl4(4);  /* task clock */

void clockIdle(void)
{
    cl0.tick();
    System_flush();
    return;
}

void msIdle(uint8_t millisecondsToIdle){
    for (uint8_t i = 0; i <= millisecondsToIdle; i++){
        cl0.tick();
    }
    System_flush();
    return;
}


Void taskFxn(UArg arg0, UArg arg1) {

    MAX30105    particleSensor;
    MCP9808     tempSensor;

    uint8_t         partID;
    uint8_t         iterate;                       // hide this iterator counter unless needed
    bool            useMAX30105Class = true, useMCP9808Class = true;


    if (useMAX30105Class) {

        if (DEBUGPRINT) {
            System_printf("Initializing the I2C bus...\n");
            System_flush();
        }


        I2C_Handle      i2c;
        I2C_Params      i2cParams;
        I2C_Params_init(&i2cParams);                        // Create I2C for usage
        i2cParams.bitRate = I2C_400kHz;                     // Change from default 100kHz to 400kHz
        // i2c = I2C_open(Board_I2C_B0P16P17, &i2cParams);     // Board_I2C_B0P16P17 -> MSP432's Pins 1.7 and 1.6 by default = UCB0SDA and UCB0SCL

        i2c = I2C_open(Board_I2C1, &i2cParams);              // Try using UCB1SDA and UCB1SCL on Pins 6.5 and 6.4
                                                            // Added in MSP_EXP432P401R.c, line ___
                                                            // and added in MSP_EXP432P401R.h around Line 91
                                                            // and added in Board.h around Line ____

        if (i2c == NULL) {
            GPIO_write(Board_LED2, Board_LED_OFF);
            GPIO_write(Board_LED1, Board_LED_ON);
            System_abort("Error Initializing I2C from main.cpp. Check I2C wiring. Bailing on program execution for now.\n");
            System_flush();
        }
        else {
            // readRegisterU8(i2c, MAX30105_PARTID, &readBufferU8, 1);
            // partID = readBufferU8;
            partID = 99;
            partID = particleSensor.readPartID(i2c);
            if (partID == 0x15) {
                if (DEBUGPRINT) {
                    System_printf("Successful I2C handshake with MAX30105 as an I2C slave device.\n");
                    System_flush();
                }

            } else {
                if (DEBUGPRINT) {
                    System_printf("PartID = %d. ", partID);
                    System_abort("That's not the MAX30105 you're talking to... Probably better to bail and sort this out!\n");
                    System_flush();
                }
            }

        }

        particleSensor.begin(i2c);

        if (DEBUGPRINT) {
            System_printf("PartID = %d using call to MAX30105::readPartID.\n", partID);
            System_flush();
        }

        particleSensor.wakeUp(i2c);

        uint8_t ledBrightness = 255; //Options: 0=Off to 255=50mA
        uint8_t sampleAverage = 1; //Options: 1, 2, 4, 8, 16, 32
        uint8_t ledMode = 3; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
        uint16_t sampleRate = 50; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
        uint16_t pulseWidth = 411; //Options: 69, 118, 215, 411
        uint16_t adcRange = 16384; //Options: 2048, 4096, 8192, 16384

        particleSensor.setup(i2c, ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

        // Check aliveness of MPL3115A2 sensor package

//        partID = readRegisterU8(i2c, Board_MPL3115A2_ADDR, 12, &partID, 1);
//
//        if (partID == 0xC4) {
//            if (DEBUGPRINT) {
//                System_printf("MPL-3115A2 PartID = %d. That's the expected WHOAMI value for this pressure sensor.!\n", partID);
//                System_flush();
//            }
//        } else {
//            if (DEBUGPRINT) {
//                System_printf("PartID = %d. That's not the expected WHOAMI value for MPL-31115A2!\n", partID);
//                System_flush();
//            }
//        }

        // Check aliveness of BNO055 sensor package

        partID = readRegisterU8(i2c, Board_BNO055_ADDR, 0, &partID, 1);
        if (partID == 0xA0) {
            if (DEBUGPRINT) {
                System_printf("BNO055 PartID = %d. That's the expected WHOAMI value for this 9-DOF sensor!\n", partID);
                System_flush();
            }

            // Run BNO055 system status and/or power-on self test routines

            uint8_t statusID = readRegisterU8(i2c, Board_BNO055_ADDR, 0x39, &statusID, 1);
            System_printf("\tBNO055 status = %d\n", statusID);
            System_flush();

        } else {
            if (DEBUGPRINT) {
                System_printf("PartID = %d. That's not the expected WHOAMI value for BNO055!\n", partID);
                System_flush();
            }
        }

        //        // Check aliveness of MPU-9250 sensor package
        //
        //        partID = readRegisterU8(i2c, Board_MPU9250_ADDR, 117, &partID, 1);
        //        if (partID == 0x71) {
        //            if (DEBUGPRINT) {
        //                System_printf("MPU-9250 PartID = %d. That's the expected WHOAMI value for this 9-DOF sensor!\n", partID);
        //                System_flush();
        //            }
        //        } else {
        //            if (DEBUGPRINT) {
        //                System_printf("PartID = %d. That's not the expected WHOAMI value for MPU-9250!\n", partID);
        //                System_flush();
        //            }
        //        }

        // Check aliveness of MCP9808 sensor package

        partID = readRegisterU8(i2c, Board_MCP9808_ADDR, 0x07, &partID, 1);
        if (partID == 0x04) {
            if (DEBUGPRINT) {
                System_printf("MCP9808 PartID = %d. That's the expected WHOAMI value for this temperature sensor!\n", partID);
                System_flush();
            }
        } else {
            if (DEBUGPRINT) {
                System_printf("PartID = %d. That's not the expected WHOAMI value for MCP9808!\n", partID);
                System_flush();
            }
        }

        // Now try to grab some pulse ox data...

        uint8_t tempReadPtr = particleSensor.getReadPointer(i2c);
        uint8_t tempWritePtr = particleSensor.getWritePointer(i2c);
        if (DEBUGPRINT) {
            System_printf("Inspecting MAX30105 read pointer: %d\n", tempReadPtr);
            System_printf("Inspecting MAX30105 write pointer: %d\n", tempWritePtr);
            System_flush();
        }

        for (iterate = 1; iterate <= 20; iterate++) {
            if (particleSensor.safeCheck(i2c, 100)){
                System_printf("Iteration: %d | Green: %d | Red: %d | NIR: %d | ", iterate, particleSensor.getGreenFIFOHead(), particleSensor.getRedFIFOHead(), particleSensor.getNirFIFOHead() );
            }
            System_printf("TempC = %f°C or %f°F | ", particleSensor.readTemperature(i2c), particleSensor.readTemperatureF(i2c));

            System_printf("MCP9808 Temp = %f°C\n", tempSensor.readTempC(i2c) );

            System_flush();
        }

        particleSensor.shutDown(i2c);


        // Close I2C Bus

        I2C_close(i2c);
        System_printf("I2C closed!\n");
        System_flush();

    } else {

        System_printf("This path is deprecated. Quitting!\n");
        System_flush();

    }

}

/*      ======== main ========      */

int main(void) {

    Task_Params taskParams;
    Semaphore_Params semParams;         // Addded from bigtime.cpp
    Clock_Params clkParams;         // Addded from bigtime.cpp

    Board_initGeneral();             // Call board init functions
    Board_initGPIO();
    Board_initI2C();

    // Construct a main task thread that does some I2C comms using MAX30105 class
    // then some raw I2C register reads to get WHOAMI values of other I2C devices

    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)taskFxn, &taskParams, NULL);

    // Construct a sampling rate thread (e.g.: this will do 250 Hz) cycling
    Task_Params_init(&taskParams);
    taskParams.arg0 = (UArg)&cl3;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task1Stack;
    Task_construct(&task1Struct, (Task_FuncPtr)clockTask, &taskParams, NULL);

    // Construct a timer thread that does BIOS_exit (terminates execution) after a set time
    taskParams.arg0 = (UArg)&cl4;
    taskParams.stack = &task2Stack;
    Task_construct(&task2Struct, (Task_FuncPtr)clockTask, &taskParams, NULL);

    /* ==== BEGIN ADDITION FROM bigtime.cpp ==== */

    /* Construct Semaphores for clock thread to pend on, initial count 1 */
    Semaphore_Params_init(&semParams);
    Semaphore_construct(&sem0Struct, 1, &semParams);
    /* Re-use default params */
    Semaphore_construct(&sem1Struct, 1, &semParams);

    /* Obtain instance handles */
    sem0Handle = Semaphore_Handle(&sem0Struct);
    sem1Handle = Semaphore_Handle(&sem1Struct);

    Clock_Params_init(&clkParams);
    clkParams.period = 250;                                 // Altering, was 100ms; used 4ms = 250Hz; now 250ms = 4 Hz
    clkParams.startFlag = true;
    clkParams.arg = (UArg)&cl1;
    Clock_construct(&clk0Struct, (Clock_FuncPtr)clockPrd, 1, &clkParams);

    // clkParams.period = 1000;                           // clockPeriod default = 1000, or 1 second
    clkParams.period = 60000;                              // test clockPeriod default = 1000 * 60 = 1 minute
    clkParams.startFlag = true;
    clkParams.arg = (UArg)&cl2;
    Clock_construct(&clk1Struct, (Clock_FuncPtr)clockPrd, 1, &clkParams);

    System_printf("Clocks are off and running!\n");

    GPIO_write(Board_LED2, Board_LED_ON);           // Turn on the Green LED to indicate the system is running; turn off at BIOS_exit

    System_printf("Starting the tagOS prototype.\nSystem provider is set to SysMin.\n"
                  "Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);

}
