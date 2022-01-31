#ifndef i2c_H
#define i2c_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_i2c.h"
#include "em_cmu.h"
#include "em_emu.h"

/**
 * @brief   Initialization of the I2C physical layer and SDA/SCL ports assignation
 */
void initI2C(void);

/**
 * @brief   Make request to slave and readback result (Hold Master Mode -- with clock stretch during measurement)
 *
 * @param   slaveAddress    The 8 bits I2C address of the slave (including the R/nW bit)
 * @param   requestCmd      Request command
 * @param   numBytesCmd     Length of the request command
 * @param   rxBuff          Receive buffer used to store sensor answer
 * @param   nymBytesRx      Length of the response message from the sensor
 * @param   success         Flag asserted if transmission has been done correclty
 * @param   app             Application context
 */
void I2C_RequestAndReadback(uint8_t slaveAddress, uint8_t *requestCmd, uint8_t numBytesCmd, uint8_t *rxBuff, uint8_t numBytesRx);

/**
 * @brief   Simple I2C read command
 *
 * @param   slaveAddress    The 8 bits I2C address of the slave (including the R/nW bit)
 * @param   rxBuff          Receive buffer used to store sensor answer
 * @param   nymBytesRx      Length of the response message from the sensor
 * @param   success         Flag asserted if transmission has been done correclty
 * @param   app             Application context
 */
void I2C_Read(uint8_t slaveAddress, uint8_t *rxBuff, uint8_t numBytes);

/**
 * @brief   Simple I2C write command without arguments
 *
 * @param   slaveAddress    The 8 bits I2C address of the slave (including the R/nW bit)
 * @param   writeCmd        Write command
 * @param   nymBytesCmd     Length of the write command
 * @param   success         Flag asserted if transmission has been done correclty
 * @param   app             Application context
 */
void I2C_WriteCmd(uint8_t slaveAddress, uint8_t *writeCmd, uint8_t numBytesCmd);

/**
 * @brief   I2C write command with arguments
 *
 * @param   slaveAddress    The 8 bits I2C address of the slave (including the R/nW bit)
 * @param   writeCmd        Write command
 * @param   settingValue    Arguments of the write command
 * @param   numBytesCmd     Length of the write command
 * @param   numBytesData    Length of the arguments
 * @param   success         Flag asserted if transmission has been done correclty
 * @param   app             Application context
 */
void I2C_WriteCmdArgs(uint8_t slaveAddress, uint8_t *writeCmd, uint8_t *settingValue, uint8_t numBytesCmd, uint8_t numBytesData);

#endif
