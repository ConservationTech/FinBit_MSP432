/*
 * modules.h
 *
 *  Created on: Aug 2, 2017
 *      Author: dave
 */

#ifndef MODULES_H_
#define MODULES_H_

#define USE_MAX30105        1
#define USE_MPU9250         1
//#define USE_BNO055          0
#define USE_MCP9808         1
#define USE_MS580314BA      1
//#define USE_KELLER          0
#define USE_SDSPI           1
//#define USE_SPIFLASH        0
//#define USE_CC31XXWIFI      0
//#define USE_ADS1298         0
//#define USE_LIPOGAUGE       0
#define USE_GPSADA        1
//#define USE_GPS_5100      0
//#define USE_FASTLOK       0

#ifdef USE_MAX30105
#include "MAX30105.h"
#endif

#ifdef USE_MCP9808
#include "MCP9808.h"
#endif

#ifdef USE_MPU9250
#include "MPU9250.h"
#endif

#ifdef USE_MS580314BA
#include "MS5803.h"
#endif

#ifdef USE_KELLER
#include "KELLER.h"
#endif

extern bool useMAX30105, useMPU9250, useBNO055,
    useMCP9808, useMS5803, useKELLER, useSDSPI, useSPIFLASH,
    useCC3120, useADS1298, useLIPOGAUGE,
    useGPS_ADA, useGPS_5100, useFASTLOK;

#endif /* MODULES_H_ */
