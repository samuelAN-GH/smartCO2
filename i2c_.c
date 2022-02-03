#include <i2c_.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_i2c.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "app.h"
#include "log.h"
#include "pt/pt.h"

#define I2C_FREQ 50000

//////////////////////////////////////////////////////////////////////

void initI2C(void)
{
  // Use default settings
  I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;
  i2cInit.freq = I2C_FREQ;

  // Using PB01 (SDA) and PB00 (SCL)
  GPIO_PinModeSet(gpioPortB, 1, gpioModeWiredAndPullUpFilter, 1);
  GPIO_PinModeSet(gpioPortB, 0, gpioModeWiredAndPullUpFilter, 1);

  // Route I2C pins to GPIO
  GPIO->I2CROUTE[0].SDAROUTE = (GPIO->I2CROUTE[0].SDAROUTE & ~_GPIO_I2C_SDAROUTE_MASK)
                        | (gpioPortB << _GPIO_I2C_SDAROUTE_PORT_SHIFT
                        | (1 << _GPIO_I2C_SDAROUTE_PIN_SHIFT));
  GPIO->I2CROUTE[0].SCLROUTE = (GPIO->I2CROUTE[0].SCLROUTE & ~_GPIO_I2C_SCLROUTE_MASK)
                        | (gpioPortB << _GPIO_I2C_SCLROUTE_PORT_SHIFT
                        | (0 << _GPIO_I2C_SCLROUTE_PIN_SHIFT));
  GPIO->I2CROUTE[0].ROUTEEN = GPIO_I2C_ROUTEEN_SDAPEN | GPIO_I2C_ROUTEEN_SCLPEN;

  // Initialize the I2C
  I2C_Init(I2C0, &i2cInit);

  // Enable automatic STOP on NACK
  I2C0->CTRL = I2C_CTRL_AUTOSN;
}

/*void I2C_RequestAndReadback(uint8_t slaveAddress, uint8_t *requestCmd, uint8_t numBytesCmd, uint8_t *rxBuff, uint8_t numBytesRx)
{
  INFO("Initializing I2C data vector for Request/Readback");

  I2C_TransferSeq_TypeDef i2cTransfer;
  I2C_TransferReturn_TypeDef result;

  // Initialize I2C transfer
  i2cTransfer.addr          = slaveAddress;
  i2cTransfer.flags         = I2C_FLAG_WRITE_READ;
  i2cTransfer.buf[0].data   = requestCmd;
  i2cTransfer.buf[0].len    = numBytesCmd;  //bytes
  i2cTransfer.buf[1].data   = rxBuff;
  i2cTransfer.buf[1].len    = numBytesRx;

  INFO("I2C Request/Readback Transfer initializing");
  result = I2C_TransferInit(I2C0, &i2cTransfer); // returns the status of on-going transfer, i2cTransferInProgress if transfer not finished

  if (I2C_Transfer(I2C0)!=i2cTransferDone) {
    ERROR("I2C transaction abort (success = false)");
  }
}*/

void I2C_Read(uint8_t slaveAddress, uint8_t *rxBuff, uint8_t numBytes)
{

  uint8_t address = (slaveAddress << 1) | 1;

  INFO("Initializing I2C data vector for Read");
  // Transfer structure
  I2C_TransferSeq_TypeDef i2cTransfer;
  I2C_TransferReturn_TypeDef result;

  // Initialize I2C transfer
  i2cTransfer.addr          = address;
  i2cTransfer.flags         = I2C_FLAG_READ;
  i2cTransfer.buf[0].data   = rxBuff;         // rxBuffer = data[numBytes]
  i2cTransfer.buf[0].len    = numBytes;       // numBytes is < I2C_TXBUFFER_SIZE

  INFO("I2C Read Transfer initializing");

  result = I2C_TransferInit(I2C0, &i2cTransfer); // returns the status of on-going transfer, i2cTransferInProgress if transfer not finished

  while (result == i2cTransferInProgress) {
    result = I2C_Transfer(I2C0);
  }

  // Error occured during transfer
  if (result != i2cTransferDone) {
    ERROR("I2C transaction abort (success = false)");
  }
}

void I2C_WriteCmd(uint8_t slaveAddress, uint8_t *writeCmd)
{
  uint8_t address = (slaveAddress << 1);
  uint8_t numBytesCmd = sizeof(writeCmd);
  INFO("Initializing I2C data vector for Write (without args)");
  I2C_TransferSeq_TypeDef i2cTransfer;
  I2C_TransferReturn_TypeDef result;

  // Initialize I2C transfer
  i2cTransfer.addr          = address;
  i2cTransfer.flags         = I2C_FLAG_WRITE;
  i2cTransfer.buf[0].data   = writeCmd;     // txBuffer = writeCmd[1byte] + data[numBytes]
  i2cTransfer.buf[0].len    = numBytesCmd; // numBytes is < I2C_TXBUFFER_SIZE
  i2cTransfer.buf[1].data   = NULL;     // txBuffer = writeCmd[1byte] + data[numBytes]
  i2cTransfer.buf[1].len    = 0; // numBytes is < I2C_TXBUFFER_SIZE

  INFO("I2C Write Transfer initializing");

  result = I2C_TransferInit(I2C0, &i2cTransfer);

  while (result == i2cTransferInProgress) {
    result = I2C_Transfer(I2C0);
  }

  if (result != i2cTransferDone) {
    ERROR("I2C transaction abort (success = false)");
  }
}

void I2C_WriteCmdArgs(uint8_t slaveAddress, uint8_t *writeCmd, uint8_t *settingValue)
{
  uint8_t numBytesData = sizeof(settingValue);
  uint8_t numBytesCmd = sizeof(writeCmd);
  uint8_t address = (slaveAddress << 1);
  INFO("Initializing I2C data vector for Write (with args)");
  I2C_TransferSeq_TypeDef i2cTransfer;
  I2C_TransferReturn_TypeDef result;

  uint8_t txBuffer[numBytesCmd + numBytesData];  // writeCmd prior data to write

  for(int i = 0; i < (numBytesCmd); i++)
  {
      txBuffer[i] = writeCmd[i];
  }
  for(int i = numBytesCmd; i < (numBytesCmd+numBytesData); i++)
  {
      txBuffer[i] = settingValue[i-numBytesCmd];  // data in txBuff is added after the writeCmd
  }

  // Initialize I2C transfer
  i2cTransfer.addr          = address;
  i2cTransfer.flags         = I2C_FLAG_WRITE;
  i2cTransfer.buf[0].data   = txBuffer;     // txBuffer = writeCmd[1byte] + data[numBytes]
  i2cTransfer.buf[0].len    = (numBytesCmd+numBytesData); // numBytes is < I2C_TXBUFFER_SIZE
  i2cTransfer.buf[1].data   = NULL;
  i2cTransfer.buf[1].len    = 0;

  INFO("I2C Write Transfer initializing");

  result = I2C_TransferInit(I2C0, &i2cTransfer);

  // Send data
  while (result == i2cTransferInProgress) {
    result = I2C_Transfer(I2C0);
  }

  if (result != i2cTransferDone) {
    ERROR("I2C transaction abort (success = false)");
  }
}
