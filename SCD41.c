#include <i2c_.h>
#include "SCD41.h"
#include "app.h"
#include <openthread/platform/alarm-milli.h>
#include "pt/pt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "coap_.h"
#include "dns.h"
#include "config_.h"
#include "log.h"

#define NOW()                 otPlatAlarmMilliGetNow()
#define PT_SLEEP(x)           PT_WAIT_UNTIL(pt, (NOW() - timeLog) >= (x))

#define SCD41_ADDRESS 0x62      // 7 bits I2C address

// SCD41 commands list
#define SCD41_GET_SN                  {0x36, 0x82}
#define SCD41_SELF_TEST               {0x36, 0x39}
#define SCD41_GET_DATA_RDY            {0xE4, 0xB8}
#define SCD41_START_LP_PERIODIC_MEAS  {0x21, 0xAC}
#define SCD41_READ_MEAS               {0xEC, 0x05}
#define SCD41_SET_ASC_ON              {0x24, 0x16}
#define SCD41_GET_ASC_ON              {0x23, 0x13}

#define SCD41_I2C_MAX_RXBUFFER_SIZE   10

static uint64_t timeLog;
uint8_t i2c_rxBuffer_scd41[SCD41_I2C_MAX_RXBUFFER_SIZE];


PT_THREAD(SCD41UpdateValueThread(struct pt *pt, bool *pThreadDone, float *co2, float *temp, float *rh))
{
  INFO("[Thread 3] Checking SCD30 data ready ...");

  PT_BEGIN(pt);

  uint8_t getDataRdy[2] = SCD41_GET_DATA_RDY;

  I2C_WriteCmd(SCD41_ADDRESS, getDataRdy, 2);

  timeLog = NOW();
  PT_SLEEP(2);

  I2C_Read(SCD41_ADDRESS, i2c_rxBuffer_scd41, 3);

  uint16_t rawRdy = (i2c_rxBuffer_scd41[7] << 8) | i2c_rxBuffer_scd41[8];

  uint16_t rdy = (rawRdy << 5) & 65504;

  if (!rdy) {
      INFO("[Thread 3] Data is not ready, leaving thread ...");
      *pThreadDone = true;
      PT_EXIT(pt);
  }

  else {
      INFO("[Thread 3] Data ready, reading ...");

      uint8_t readMeas[2] = SCD41_READ_MEAS;
      I2C_WriteCmd(SCD41_ADDRESS, readMeas, 2);

      timeLog = NOW();
      PT_SLEEP(2);

      I2C_Read(SCD41_ADDRESS, i2c_rxBuffer_scd41, 9);

      *co2 = (float)((i2c_rxBuffer_scd41[1] << 8) | i2c_rxBuffer_scd41[2]);
      *temp = (float)(-45 + 175 * ((i2c_rxBuffer_scd41[4] << 8) | i2c_rxBuffer_scd41[5]));
      *rh = (float)(100*((i2c_rxBuffer_scd41[7] << 8) | i2c_rxBuffer_scd41[8]));

      INFO("[Thread 3] Data read done.");
  }

  *pThreadDone = true;
  PT_END(pt);
}
