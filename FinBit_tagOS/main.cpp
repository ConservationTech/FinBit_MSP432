 /*
 *    MAX30105 over i2c by Dave Haas - created on 17 June 2017
 *    Based an i2c example from TI for MSP432 LaunchPad.
 *    Code to address the MAX30105 module was copied from
 *    MAX30105.h and MAX30105.cpp by Peter Jansen and Nathan Seidle (SparkFun).
 *    This code and all code for MAX30105 are governed under the
 *    BSD license, so all text above must be included in any redistribution.
 */

/* XDCtools Header files */
#include <xdc/runtime/System.h>
#include <xdc/runtime/Diags.h>
#include <xdc/std.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SDSPI.h>

/* Standard Library for handling file i/o, concatentation, etc */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* Include the generic I2C helper function prototypes */
#include "i2c_helper.h"

/* Include system_clock.h for clock and time functions */
#include "system_clock.h"

/* Example/Board Header files */
#include "Board.h"

/* Include the MAX30105 header file */
//#include "MAX30105.h"

/* Include the MCP9808 header file */
//#include "MCP9808.h"

/* Include the MS5803 header file */
// #include "MS5803.h"

/* DEFINE WHICH MSP432 INTERFACES TO BUILD AND USE */
#include "interfaces.h"

#ifdef USE_I2C
    bool useI2C = true;
#else
    bool useI2C = false;
#endif

#ifdef USE_SPI
    bool useSPI = true;
#else
    bool useSPI = false;
#endif

/* DEFINE WHICH SENSORS TO BUILD AND USE */

#include "modules.h"

#ifdef USE_MAX30105
    bool useMAX30105 = true;
#else
    bool useMAX30105 = false;
#endif

#ifdef USE_MPU9250
    bool useMPU9250 = true;
#else
    bool useMPU9250 = false;
#endif

#ifdef USE_BNO055
    bool useBNO055 = true;
#else
    bool useBNO055 = false;
#endif

#ifdef USE_MCP9808
    bool useMCP9808 = true;
#else
    bool useMCP9808 = false;
#endif

#ifdef USE_MS580314BA
    bool useMS5803 = true;
#else
    bool useMS5803 = false;
#endif

#ifdef USE_SDSPI
    bool useSDSPI = true;
#else
    bool useSDSPI = false;
#endif

#ifdef USE_SPIFLASH
    bool useSPIFLASH = true;
#else
    bool useSPIFLASH = false;
#endif

#ifdef USE_CC31XXWIFI
    bool useCC3120 = true;
#else
    bool useCC3120 = false;
#endif

#ifdef USE_ADS1298
    bool useADS1298 = true;
#else
    bool useADS1298 = false;
#endif

#ifdef USE_LIPOGAUGE
    bool useLIPOGAUGE = true;
#else
    bool useLIPOGAUGE = false;
#endif

#ifdef USE_GPS_ADA
    bool useGPS_ADA = true;
#else
    bool useGPS_ADA = false;
#endif
#ifdef use_GPS_5100
    bool useGPS_5100 = true;
#else
    bool useGPS_5100 = false;
#endif

#ifdef USE_FASTLOK
    bool useFASTLOK = true;
#else
    bool useFASTLOK = false;
#endif

/* DEFINE BEHAVIOR OF VERBOSE DEBUG AND ALIVENESS System_printf FUNCTIONALITY THROUGHOUT CODE */

#include "condPrintf.h"

/* Enable conditional debug printing depending on define */

#ifdef DEBUGPRINT                 /* THIS GETS DEFINED IN condPrint.h  - go there to enable/disable verbose debug printf behavior */
    bool debugPrint = true;
#else
    bool debugPrint = false;
#endif

#ifdef POSTPRINT                 /* THIS GETS DEFINED IN condPrint.h  - go there to enable/disable aliveness/POST printf behavior */
    bool postPrint = true;
#else
    bool postPrint = false;
#endif


/* Buffer size used for the file copy process */
#define FATSD_BUFF_SIZE       2048

/* String conversion macro */
#define STR_(n)             #n
#define STR(n)              STR_(n)

/* Drive number used for FatFs */
#define DRIVE_NUM           0

#define TASKSTACKSIZE       8192         // Default was 640 | fatsd was 768 | now testing 4096 and 6142


void clockIdle(void);

Task_Struct task0Struct, task1Struct, task2Struct;      // Added task1Struct &task2Struct for clocks
Char task0Stack[TASKSTACKSIZE], task1Stack[TASKSTACKSIZE], task2Stack[TASKSTACKSIZE];

Semaphore_Struct sem0Struct, sem1Struct;
Semaphore_Handle sem0Handle, sem1Handle;
Clock_Struct clk0Struct, clk1Struct;

/* Global clock objects */
Clock cl0(0);  /* idle loop clock */
Clock cl1(1);  /* periodic clock, period = 5 ms or 200 Hz */
Clock cl2(2);  /* periodic clock, period = 1 min */
Clock cl3(3);  /* task clock */
Clock cl4(4);  /* task clock */

// const char fileName[]     = "fat:"STR(DRIVE_NUM)":tagOS1.bin";

const uint8_t structSize = 128;

struct fatsdWriteBuffer {
    // uint32_t iteration[structSize];
    uint32_t red[structSize];
    uint32_t nir[structSize];
    float dieTempC[structSize];
    float ambientTempC[structSize];
} writeBuffer, readBuffer;

struct Date {
    uint8_t century, year, month, day, hour, minute, second;
} timeStamp;


void clockIdle(void) {
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

    char tempText[512]          = "";
    char textBuffer[4096]       = "";

    /* Instantiate classes for main task function */

    MAX30105        particleSensor;
    MCP9808         tempSensor;

    if (debugPrint){
        System_printf("*** tagOS v0.1a ***\n");
        System_flush();
    }

    /* Populate the main task's timeStamp structure */

    timeStamp.century   = cl2.getCentury();
    timeStamp.year      = cl2.getYear();
    timeStamp.month     = cl2.getMonth();
    timeStamp.day       = cl2.getDay();
    timeStamp.hour      = cl2.getHour();
    timeStamp.minute    = cl2.getMinute();
    timeStamp.second    = cl2.getSecond();

    System_printf("\tRecord start = %d%d-%.2d-%.2d %.2d:%.2d:%.2d\n", timeStamp.century, timeStamp.year,
                  timeStamp.month, timeStamp.day, timeStamp.hour, timeStamp.minute, timeStamp.second);
    System_flush();


    /* Initialize SDSPI -- this enables SD card operations over SPI */

    SDSPI_Handle    sdspiHandle;
    SDSPI_Params    sdspiParams;

    /* Variables for the CIO functions */
    FILE *src, *dst;

    /* Variables to keep track of the file copy progress */
    unsigned int bytesRead = 0;
    unsigned int bytesWritten = 0;
    unsigned int fileSizeMultiplier = 0;

    /* Mount and register the SD Card */
    SDSPI_Params_init(&sdspiParams);
    sdspiHandle = SDSPI_open(Board_SDSPI0, DRIVE_NUM, &sdspiParams);

    if (sdspiHandle == NULL) {
        System_abort("\tError initializing the SD card over SPI.\n");
    } else {
        System_printf("\tNOTE: THIS DOESN'T ACTUALLY DO ANYTHING! FIX THIS! Drive %u is mounted using SD over SPI.\n", DRIVE_NUM);
        System_flush();
    }

    /* Initialize I2C -- this enables I2C for use with I2C-based sensor packages */


    if (debugPrint) {
            System_printf("\tInitializing the I2C bus...\n");
            System_flush();
    }

    I2C_Handle      i2c;
    I2C_Params      i2cParams;
    I2C_Params_init(&i2cParams);                        // Create I2C for usage
    i2cParams.bitRate = I2C_400kHz;                     // Change from default 100kHz to 400kHz

    /* Use UCB1SDA and UCB1SCL on Pins 6.5 and 6.4 | MSP_EXP432P401R.c, line ___ | MSP_EXP432P401R.h, line 91 | Board., Line ____ */
    // i2c = I2C_open(Board_I2C_B0P16P17, &i2cParams);     // Board_I2C_B0P16P17 -> MSP432's Pins 1.7 and 1.6 by default = UCB0SDA and UCB0SCL
    i2c = I2C_open(Board_I2C1, &i2cParams);

    if (i2c == NULL) {
        GPIO_write(Board_LED2, Board_LED_OFF);
        GPIO_write(Board_LED1, Board_LED_ON);
        System_abort("\tError Initializing I2C from main.cpp. Check I2C wiring. Bailing on program execution for now.\n");
        System_flush();
    } else {
        System_printf("\tI2C bus successfully initialized.\n");
        System_flush();
    }


    /* Run sensor aliveness checks - now functionalized in i2c_helper.cpp */
    if (postPrint){
        uint8_t badSensorCount = runSensorChecks(i2c);
        if (badSensorCount == 0){
            System_printf("All I2C sensors presented and accounted for!\n");
            System_flush();
        } else {
            System_printf("At least %d I2C sensor(s) is/are not responding to I2C WHOAMI request(s).\n",
                          badSensorCount);
            System_abort("Enable debugging, fire up the o'scope, and investigate. Bailing!\n");
            System_flush();
        }
    }

    if (debugPrint){
        System_printf("\nPreparing sensors and non-volatile memory for sampling phase...\n");
        System_flush();
    }


    /* Initialize the MAX30105 module */

    uint8_t ledBrightness = 128; //Options: 0=Off to 255=50mA
    uint8_t sampleAverage = 1; //Options: 1, 2, 4, 8, 16, 32
    uint8_t ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
    uint16_t sampleRate = 200; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200 -- here 200, which performs 100Hz sampling on red and nir
    uint16_t pulseWidth = 411; //Options: 69, 118, 215, 411
    uint16_t adcRange = 16384; //Options: 2048, 4096, 8192, 16384

    if (useMAX30105){

//        particleSensor.softReset(i2c);
//        Task_sleep(5);
        particleSensor.begin(i2c);
        particleSensor.wakeUp(i2c);

        particleSensor.setup(i2c, ledBrightness, sampleAverage, ledMode, sampleRate,
                             pulseWidth, adcRange); //Configure sensor with these settings

    }

    /*
    //      Grab some pulse ox, temperature, and other sensor data...
    */

    /* Create a new file object for the file write */

    /* Create this tagOS session's dynamic file name from date and time */

    // char tempFileName[30];                                                       // max file size including final NULL
    // sprintf(tempFileName, "fat:%d:tagOS_%d%d%.2d%.2d_%.2d%.2d.bin\0",
    //            DRIVE_NUM, timeStamp.century, timeStamp.year, timeStamp.month,
    //            timeStamp.day, timeStamp.hour, timeStamp.minute);
    // const char* constFileName = tempFileName;                                    // if this gives f'd up results, switch to next line

    const char* constFileName = "fat:0:tagOS1.bin\0";                               // if above line fails, switch to this one and be done with it!

    dst = fopen(constFileName, "wb+");
    if (!dst) {
        if (debugPrint){
            System_printf("\tAttempting to create fileName: %s\n", constFileName);
            System_flush();
        }

        /* If file doesn't exist, create it and start writing */
        dst = fopen(constFileName, "wb+");
        if (!dst) {
            System_abort("\tFile creation failed! Probably best to bail and sort out now.\n");
        } else {
            if (debugPrint) {
                System_printf("\tFile created and ready for write!\n");
                System_flush();
            }
        }
    }
    else {
        // This condition probably shouldn't happen with fopen "wb+" fopen above. "wb+" = create and overwrite. But catch it anyway...
        if (debugPrint){
            System_printf("\tFile '%s' is already present, being reopened instead of created.\n", constFileName);
            System_flush();
        }
    }

    /* START SAMPLING LOOP HERE */
    if (debugPrint){
        System_printf("\nStarting sampling now.\n");
        System_flush();
    }

    /* Do some number of iterations to gather some sample data */

    uint16_t writeBufferPointer = 0;
    uint32_t writeBufferSize    = 0;
    uint32_t testTimer          = 0;
    uint32_t totalBytesWritten  = 0;

    uint32_t sampleTime         = 25;                                   // Define sample time in minutes
    uint32_t sampleDuration     = sampleTime * 1000;                   // Redefine minutes as milliseconds and call it 'sampleDuration'

    GPIO_write(Board_LED2, Board_LED_OFF);
    GPIO_write(Board_LED3, Board_LED_ON);
    while (testTimer < sampleDuration) {                                    // Sample for 'sampleDuration milliseconds, e.g.: 1000 = 1000 ms = 1 second

        cl1.tick();                                                         // Do a cl1.tick(), which should be every 10 milliseconds
        testTimer += 1;                                                     // Increment a tick accumulator with every clock tick

        // TO-DO: add a sampling delay timer to allow sensor ADCs to settle

        if ( !(testTimer % 5) )                                             // When millisecond count is divisible by 5, or 200Hz (1000 / 5 = 200)
        {

//            System_printf("testTimer = %d\n", testTimer);
//            System_flush();

            uint32_t    thisGreen, thisRed, thisNir;
            // uint32_t     pressure, pTemp, aX, aY, aZ, mX, mY, mZ, gX, gY, gZ;
            float       dieTempC, ambientTempC;

            /* 1. Grab pulse ox, temp, and any other sensor data */

            if (useMAX30105){
                if (particleSensor.safeCheck(i2c, 100)){
                    if (ledMode == 3)
                        thisGreen   = particleSensor.getGreenFIFOHead();
                        //thisGreen   = particleSensor.getGreen(i2c);

                    thisRed     = particleSensor.getRedFIFOHead();
                    //thisRed     = particleSensor.getRed(i2c);
                    thisNir     = particleSensor.getNirFIFOHead();
                    //thisNir     = particleSensor.getIR(i2c);
                    dieTempC    = particleSensor.readTemperature(i2c);
                }
            }

            if (useMCP9808){
                /* Grab ambient temperature data */
                ambientTempC    = tempSensor.readTempC(i2c);
            }

            if (useMS5803){
                /* Get pressure and pressure transducer temp */
                /* TO-DO: write the MS5803-14BA and Keller I2C drivers */
                // pressure    = pressureSensor.readPressure(i2c);
                // pTemp       = pressureSensor.readTemp(i2c);
                /* Do pressure w/ pTemp calibration and write compensated,
                 *  either here or post-collection (during read -> wi-fi transfer -- probably best)
                 */
            }

            if (useMPU9250) {
                /* Get 9-DOF sensor data */
                /* TO-DO: pass in 9-DOF data; write the MPU-9250 driver */
            }

//            if (debugPrint){
//                System_printf("\t1. Data sampled during testTimer %d.\n", testTimer);
//                System_flush();
//            }


            /* Some steps for buffering data and writing it to sd card                                  */
            /* Recall defined FATSD_BUFF_SIZE, which for large SD cards we'll assume is 2k write blocks */

            /* 2. Calculate the total size of the above data reads = currentDataSizing (should all be a constant 4 bytes in size) */
            uint8_t sizeThisRed         = sizeof(thisRed);
            uint8_t sizeThisNir         = sizeof(thisNir);
            uint8_t sizeDieTempC        = sizeof(dieTempC);
            uint8_t sizeAmbientTempC    = sizeof(ambientTempC);
//            uint8_t sizePressure        = sizeof(pressure);
//            uint8_t sizePTemp           = sizeof(pTemp);
//            uint8_t sizeAX            = sizeof(aX);
//            uint8_t sizeAY            = sizeof(aY);
//            uint8_t sizeAZ            = sizeof(aZ);
//            uint8_t sizeMX            = sizeof(mX);
//            uint8_t sizeMY            = sizeof(mY);
//            uint8_t sizeMZ            = sizeof(mZ);
//            uint8_t sizeGX            = sizeof(gX);
//            uint8_t sizeGY            = sizeof(gY);
//            uint8_t sizeGZ            = sizeof(gZ);
//            uint16_t sizeWriteBuffer     = sizeof(writeBuffer);

//            if (debugPrint){
//                System_printf("\t2. Data sized.\n");
//                System_flush();
//            }


            /* 3. Calculate data packet sizing */
            /* Get rid of this eventually, move to manually setting dataPacketSize once all sensors are config'd & sizing is known */

            uint8_t dataPacketSize = sizeThisRed + sizeThisNir + sizeDieTempC + sizeAmbientTempC;
//            uint8_t flushInterval  = FATSD_BUFF_SIZE / (uint16_t)dataPacketSize;

            /* 3. Concatenate the above data reads into writeBuffer for later fatsd file write */

            writeBuffer.red[writeBufferPointer]             = thisRed;
            writeBuffer.nir[writeBufferPointer]             = thisNir;
            writeBuffer.dieTempC[writeBufferPointer]        = dieTempC;
            writeBuffer.ambientTempC[writeBufferPointer]    = ambientTempC;
//            writeBuffer.pressure[writeBufferPointer]        = pressure;
//            writeBuffer.pTemp[writeBufferPointer]           = pTemp;
//            writeBuffer.ax[writeBufferPointer]              = aX;
//            writeBuffer.ay[writeBufferPointer]              = aY;
//            writeBuffer.az[writeBufferPointer]              = aZ;
//            writeBuffer.mx[writeBufferPointer]              = mX;
//            writeBuffer.my[writeBufferPointer]              = mY;
//            writeBuffer.mz[writeBufferPointer]              = mZ;
//            writeBuffer.gx[writeBufferPointer]              = gX;
//            writeBuffer.gy[writeBufferPointer]              = gY;
//            writeBuffer.gz[writeBufferPointer]              = gZ;

//            if (debugPrint) {
//                System_printf("\t3. Data stored in writeBuffer.\n");
//                System_flush();
//            }


            /* 4. Increment the writeBufferPointer. When it hits flushInterval (~2k), write to sd card and reset writeBufferPointer to 0 */

            writeBufferPointer  += 1;               // Increment writeBufferPointer every time data is added to buffer
            writeBufferSize     += dataPacketSize;  // Increment writeBufferSize every time data is added to buffer, based on size of data passed in
//            if (debugPrint) {
//                System_printf("\twriteBufferPointer: %d | writeBufferSize: %d\n", writeBufferPointer, writeBufferSize);
//                System_flush();
//            }

            // uint8_t flushTime = iterate % 128;   // ITERATE GOES AWAY W/O FOR LOOP / DURING WHILE LOOP -- FIX THIS!!!!

            /* 5. Do write before writeBufferSize > FATSD_BUFF_SIZE */

            // if ( (flushTime == 0 ) && (writeBufferPointer == 128) ) {            // I.e.: when the writeBuffer has 128 elements = 2048 bytes = 1 sd card block
            // if ( !(testTimer % 100) ) {                                          // Try using only (!(testTimer % 100))  // i.e.: when 100 ms have passed, do the following

            if ( (writeBufferSize == 2048) || (writeBufferSize + ((uint32_t)dataPacketSize) > 2048)  ) {         // If next data aggregate will cause buffer > 2k, write buffer now

                /* Time to write the writeBuffer to non-volatile memory! */
                // bytesRead = 2048;                                               // Adjust this later, so that it happens dynamically based on sizeof() sensor data samples
                // bytesWritten = fwrite(&writeBuffer, 1, bytesRead, dst);

                bytesWritten = fwrite(&writeBuffer, 1, writeBufferSize, dst);
                uint8_t sdFlush = fflush(dst);

                if (bytesWritten > 0) {                                             // If at least one byte is written to nvm...

                    /* increment fileSizeMultiplier, then reset the writeBufferPointer, so no buffer overflowing! */
                    totalBytesWritten   += (uint32_t)bytesWritten;                  // Compute total bytes committed to nvm
                    fileSizeMultiplier  += 1;                                       // Number of samples written to sd / non-volatile memory

//                    if (debugPrint){
//                        System_printf("\tTick: %d | Bytes written: %d | Bytes read: %d\n\t\ttotalBytesWritten: %d | flushReturn: %d | fileSizeMultiplier: %d\n",
//                                                         testTimer, bytesWritten, writeBufferSize, totalBytesWritten, sdFlush, fileSizeMultiplier);
//                        System_flush();
//                    }

                    writeBufferPointer  = 0;                        // Reset the writeBufferPointer
                    writeBufferSize     = 0;                        // Reset the writeBufferSize (avoids endless conditional writing)

                }

                if (bytesWritten < bytesRead) {
                    /* Error or Disk Full */
                    System_printf("\t*** Disk Full error. Something's not working right... Investigate! ***\n");
                    System_flush();
                }


            }

//            if (debugPrint) {
//                if (ledMode == 3)
//                    System_printf("\t\tIteration: %d | Green: %d | Red: %d | NIR: %d | dietempC: %f°C | ambientTempC: %f°C\n",
//                              testTimer, thisGreen, thisRed, thisNir, dieTempC, ambientTempC);
//                else if (ledMode == 2)
//                    System_printf("\t\tIteration: %d | Red: %d | NIR: %d | dietempC: %f°C | ambientTempC: %f°C\n",
//                              testTimer, thisRed, thisNir, dieTempC, ambientTempC);
//                    System_printf("\t\tData Sizings = %d %d %d %d %d %d %d \n",
//                                  sizeThisRed, sizeThisNir, sizeDieTempC, sizeAmbientTempC, sizeWriteBuffer, writeBufferPointer, fileSizeMultiplier);
//
//                System_flush();
//            }

        }

    }

    /* END SAMPLING LOOP HERE */

    if ( (writeBufferSize <= 2048) && (writeBufferSize > 0)  ) {         // If unwritten data remains in write buffer, write it!

       /* Time to write the writeBuffer to non-volatile memory! */
       // bytesRead = 2048;                                               // Adjust this later, so that it happens dynamically based on sizeof() sensor data samples
       // bytesWritten = fwrite(&writeBuffer, 1, bytesRead, dst);

       bytesWritten = fwrite(&writeBuffer, 1, writeBufferSize, dst);
       uint8_t sdFlush = fflush(dst);

       if (bytesWritten > 0) {                                             // If at least one byte is written to nvm...

           /* increment fileSizeMultiplier, then reset the writeBufferPointer, so no buffer overflowing! */
           totalBytesWritten   += (uint32_t)bytesWritten;                  // Compute total bytes committed to nvm
           fileSizeMultiplier  += 1;                                       // Number of samples written to sd / non-volatile memory

           if (debugPrint){
               System_printf("\tTick: %d | Bytes written: %d | Bytes read: %d\n\t\ttotalBytesWritten: %d | flushReturn: %d | fileSizeMultiplier: %d\n",
                                                testTimer, bytesWritten, writeBufferSize, totalBytesWritten, sdFlush, fileSizeMultiplier);
               System_flush();
           }

           writeBufferPointer  = 0;                        // Reset the writeBufferPointer
           writeBufferSize     = 0;                        // Reset the writeBufferSize (avoids endless conditional writing)

       }
    }


    System_printf("\n\nSampling ended. Here's the stats:\n");
    System_printf("\ttestTimer end (ms): %d | totalBytesWritten: %d | fileSizeMultiplier: %d\n", testTimer, totalBytesWritten, fileSizeMultiplier);
    System_flush();

    System_printf("\nTransitioning to 'off-animal' mode'.\n");
    System_flush();

    /* Shutdown MAX30105 */
    System_printf("\tQuieting sensors...\n");
    System_flush();
    particleSensor.shutDown(i2c);

    /* Close I2C Bus */
    I2C_close(i2c);
    System_printf("\tI2C bus closed!\n");
    System_flush();

    /* Close the file handle */
    uint8_t closeFile = fclose(dst);
    System_printf("\tFinished writing data to SD. fclose: #%d.\n", closeFile);
    System_flush();

    /* Flush and close SDSPI Bus */
    SDSPI_close(sdspiHandle);
    System_printf("\tDrive %u unmounted\n\n", DRIVE_NUM);

    /*
     *  TRANSITION TO DRY-MODE -- OFF-ANIMAL and ON-CHARGER
     */

    GPIO_write(Board_LED3, Board_LED_OFF);
    GPIO_write(Board_LED2, Board_LED_ON);
    /* Reopen SDSPI and do a read of data */

    /* Variables to keep track of the file copy progress */
    bytesRead = 0;
    bytesWritten = 0;

    /* Mount and register the SD Card */
    SDSPI_Params_init(&sdspiParams);
    sdspiHandle = SDSPI_open(Board_SDSPI0, DRIVE_NUM, &sdspiParams);

    if (sdspiHandle == NULL) {
        System_abort("Error starting the SD card\n");
    } else {
        System_printf("Entering SD Card read phase.\nDrive %u was successfully mounted.\n", DRIVE_NUM);
        System_flush();
    }

    /* Try to open the source file */
    src = fopen(constFileName, "rb");
    if (!src) {
        System_printf("File \"%s\" is not present on SD Card...\n", constFileName);
        System_flush();
    } else {
        System_printf("File %s was opened for read.\n", constFileName);
        System_flush();
    }

    // uint32_t bytesToRead = FATSD_BUFF_SIZE * fileSizeMultiplier;
    uint32_t bytesToRead = totalBytesWritten;

    System_printf("\tbytesToRead should be: %d.\n", bytesToRead);
    System_flush();

    /* Try to open the tab-delimited text file we'll write all data to */

    const char textFileName[] = "fat:0:tagOS1.txt\0";                               // if above line fails, switch to this one and be done with it!

    dst = fopen(textFileName, "w+");
    Task_sleep(5);
    if (!dst) {
        /* If file doesn't exist, create it and start writing */
        System_printf("\tAttempting to create fileName: %s\n", textFileName);
                System_flush();
        dst = fopen(textFileName, "w+");
        if (!dst) {
            System_abort("\tFile creation failed! Probably best to bail and sort out now.\n");
        } else {
            System_printf("\tFile created and ready for write!\n");
            System_flush();
        }
    }
    else {
        // This condition probably shouldn't happen with fopen "w+" fopen above. "w+" = create and overwrite. But catch it anyway...
        System_printf("\t*** File '%s' is already present. It should've been created from scratch, but is being reopened instead. Investigate! ***\n", textFileName);
        System_flush();
    }

    uint32_t sdBlock = 0;                   // Initialize sd card block count

    uint32_t textBytesWritten   = 0;        // Initialize a new byte counter for number of text file conversion bytes written
    uint32_t totalBytesRead     = 0;        // ... and another to keep track of totalBytesRead (for reporting purposes)
    writeBufferPointer          = 0;        // Re-init existing writeBufferPointer
    writeBufferSize             = 0;        // Re-init existing writeBufferSize
//    char tempText[512]          = "";
//    char textBuffer[4096]       = "";

    /* Add a text header to the start of the textBuffer */
    sprintf(tempText, "red\tnir\tdieTempC\tambientTemp\n");
    strcpy(textBuffer, tempText);

    do {

        bytesRead = fread(&readBuffer, 1, FATSD_BUFF_SIZE, src);
        uint32_t readPosition = ftell(src);

        bytesToRead = bytesToRead - bytesRead;

        for (uint8_t i = 0; i <= 127; i++) {                        // CHANGED TO LESS THAN OR EQUAL TO ON 3 AUG 2017 - DKH
            writeBuffer.red[i] = readBuffer.red[i];
            writeBuffer.nir[i] = readBuffer.nir[i];
            writeBuffer.dieTempC[i] = readBuffer.dieTempC[i];
            writeBuffer.ambientTempC[i] = readBuffer.ambientTempC[i];

            sprintf(tempText, "%d\t%d\t%f\t%f\n\0", writeBuffer.red[i], writeBuffer.nir[i], writeBuffer.dieTempC[i], writeBuffer.ambientTempC[i] );
            //snprintf(tempText, "%d\t%d\t%f\t%f\n", writeBuffer.red[i], writeBuffer.nir[i], writeBuffer.dieTempC[i], writeBuffer.ambientTempC[i] );

            if (sdBlock == 0 && i == 0 ) {
                strcat(textBuffer, tempText);
            } else if (sdBlock > 0 && i == 0) {
                strcpy(textBuffer, tempText);
            } else {
                strcat(textBuffer, tempText);
            }

//            System_printf("%s\n", tempText);
//            System_printf("%s\n", textBuffer);
//            System_flush();

        }

        uint16_t textBufferSize = sizeof(textBuffer);

        if ( (textBufferSize >= 3840) && (textBufferSize <= 4096) ) {
            // Append a null to the end of the text string being written...
            strcat(textBuffer, "\0");
            // Write to file when bytes between 3840 and 4096
            bytesWritten = fwrite(&textBuffer, 1, textBufferSize, dst);
        }

        strcpy(tempText, "");
        strcpy(textBuffer, "");

        textBytesWritten += (uint32_t)bytesWritten;

//        System_printf("fread stats :: | bytesRead: %d | bytesToRead: %d | readPos: %d\n", bytesRead, bytesToRead, readPosition);
//        System_flush();

//        for (uint8_t j = 0; j <= 127; j+=64) {
//
//            System_printf("\tdata spot-check :: | sdBlock: %d | Var: %d | Red: %d | NIR: %d | dieTempC: %f | ambientTempC: %f\n",
//                          sdBlock, j, readBuffer.red[j], readBuffer.nir[j], readBuffer.dieTempC[j], readBuffer.ambientTempC[j]);
//            System_flush();
//        }

        sdBlock += 1;
        totalBytesRead += bytesRead;

    } while (bytesToRead > 0);

    System_printf("Conversion to text stats :: Bytes read: %d | Bytes of text written: %d\n", totalBytesRead, textBytesWritten);
    System_flush();

    closeFile = fclose(src);
    System_printf("Finished reading data from SD. fclose: #%d.\n", closeFile);

    uint8_t closeTextFile = fclose(dst);
    System_printf("Finshed writing text data to SD. fclose: #%d.\n", closeTextFile);
    System_flush();

    /* Flush and close SDSPI Bus */
    SDSPI_close(sdspiHandle);
    System_printf("Drive %u unmounted\n", DRIVE_NUM);

    /* End record timestamp */
    timeStamp.century   = cl2.getCentury();
    timeStamp.year      = cl2.getYear();
    timeStamp.month     = cl2.getMonth();
    timeStamp.day       = cl2.getDay();
    timeStamp.hour      = cl2.getHour();
    timeStamp.minute    = cl2.getMinute();
    timeStamp.second    = cl2.getSecond();

    System_printf("Record end = %d%d-%.2d-%.2d %.2d:%.2d:%.2d\n", timeStamp.century, timeStamp.year,
                  timeStamp.month, timeStamp.day, timeStamp.hour, timeStamp.minute, timeStamp.second);
    System_flush();

}

/*      ======== main ========      */

int main(void) {

    Task_Params taskParams;
    Semaphore_Params semParams;         // Addded from bigtime.cpp
    Clock_Params clkParams;         // Addded from bigtime.cpp

    Board_initGeneral();             // Call board init functions
    Board_initGPIO();
    Board_initI2C();
    Board_initSDSPI();              // 21Jul - DKH - Added for microSD_FatFS_write
//    Board_initWiFi();               // 5 Aug - DKH - Added for CC3120 WiFi enable

    // Construct a main task thread that does some I2C comms using MAX30105 class
    // then some raw I2C register reads to get WHOAMI values of other I2C devices

    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)taskFxn, &taskParams, NULL);

    // Construct a sampling rate thread - this will do 250 Hz sampling; clock speed determined below (see cl1)
    Task_Params_init(&taskParams);
    taskParams.arg0 = (UArg)&cl3;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task1Stack;
    Task_construct(&task1Struct, (Task_FuncPtr)clockTask, &taskParams, NULL);

    // Construct a timer thread that does BIOS_exit (terminates execution) after a set time - determined below (see cl2)
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

    /* set cl1 tickTime = 10ms or 100Hz */
    clkParams.period = 1;                                 // 4ms = 250 Hz | 5ms = 200Hz | 10ms = 100Hz | 250ms = 4 Hz
    clkParams.startFlag = true;
    clkParams.arg = (UArg)&cl1;
    Clock_construct(&clk0Struct, (Clock_FuncPtr)clockPrd, 1, &clkParams);

    /* set cl2 tickTime = 60000 ms or 1 minute */
    clkParams.period = 60000;                             // clockPeriod default = 1000 = 1 second | 1000 * 60 = 60000 = 1 minute
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
