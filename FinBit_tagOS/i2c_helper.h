/*
 * i2c_helper.h
 *
 *  Created on: Jun 29, 2017
 *      Author: dave
 */

#ifndef I2C_HELPER_H_
#define I2C_HELPER_H_

#define Board_MAX30105_ADDR         (0x57)                  // Sparkfun MAX30105 Brealout  - Particle Sensor LED + Photo-receptor Package
#define Board_MPU9250_ADDR          (0x68)                  // Sparkfun MPU-9250 Breakout  - 9-DOF Accel + Mag + Gyro IMU Sensor Package
#define Board_MPL3115A2_ADDR        (0x60)                  // Sparkfun MPL3115A2 Breakout - Altitude and Temperature (??) Sensor Package
#define Board_ADS1115_ADDR          (0x48)                  // Adafruit ADS1115 Breakout   - ADS1115 16-bit A0-A4 ADC with I2C Interface
#define Board_MCP9808_ADDR          (0x18)                  // Adafruit MCP9808 Breakout   - MCP9808 Temperature Sensor

int8_t readRegisterS8(I2C_Handle handle, uint8_t devAddr, uint8_t regAddr, int8_t *data, size_t length);
uint8_t readRegisterU8(I2C_Handle handle, uint8_t devAddr, uint8_t regAddr, uint8_t *data, size_t length);
void writeRegister8(I2C_Handle handle, uint8_t devAddr, uint8_t regAddr, uint8_t value);
void writeRegister8R(I2C_Handle handle, uint8_t devAddr, uint8_t regAddr, uint8_t value, uint8_t readReg);

#endif /* I2C_HELPER_H_ */
