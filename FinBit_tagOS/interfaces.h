/*
 * interfaces.h
 *
 *  Created on: Aug 2, 2017
 *      Author: dave
 */

#ifndef INTERFACES_H_
#define INTERFACES_H_

/* Comment out these defines to disable MSP432 interfaces
 * Think of these as boolean values, i.e.: 0 = false, 1 = true
 */

#define USE_I2C     1
#define USE_SPI     1
#define USE_PWM     0

extern bool useI2C;
extern bool useSPI;
extern bool usePWM;

#endif /* INTERFACES_H_ */
