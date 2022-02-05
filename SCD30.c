#include <i2c_.h>
#include "SCD30.h"
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

#define SCD30_ADDRESS 0x61      // 7 bits I2C address

// SCD41 commands list
#define SCD30_GET_FW                   {0xD1, 0x00}
#define SCD30_SELF_TEST                {0x36, 0x39}
#define SCD30_GET_DATA_RDY             {0x02, 0x02}
#define SCD30_START_PERIODIC_MEAS      {0x00, 0x10} // Need args (p compensation)
#define SCD30_READ_MEAS                {0x03, 0x00}
#define SCD30_SET_ASC_ON_OFF           {0x53, 0x06} // Need args (1 or 0)

#define SCD30_MAX_RXBUFFER_SIZE   10

static uint64_t timeLog;
uint8_t i2c_rxBuffer_scd30[SCD30_MAX_RXBUFFER_SIZE];


PT_THREAD(SCD30UpdateValueThread(struct pt *pt, bool *pThreadDone, float *co2, float *temp, float *rh))
{
  INFO("[Thread 2] Checking SCD30 data ready ...");

  PT_BEGIN(pt);

  uint8_t getDataRdy[2] = SCD30_GET_DATA_RDY;

  I2C_WriteCmd(SCD30_ADDRESS, getDataRdy, 2);

  timeLog = NOW();
  PT_SLEEP(4);

  I2C_Read(SCD30_ADDRESS, i2c_rxBuffer_scd30, 3);

  uint16_t rawRdy = (i2c_rxBuffer_scd30[7] << 8) | i2c_rxBuffer_scd30[8];

  uint16_t rdy = (rawRdy << 5) & 65504;

  if (!rdy) {
      INFO("[Thread 2] Data is not ready, leaving thread ...");
      *pThreadDone = true;
      PT_EXIT(pt);
  }

  else {
      INFO("[Thread 2] Data ready, reading ...");

      uint8_t readMeas[2] = SCD30_READ_MEAS;
      I2C_WriteCmd(SCD30_ADDRESS, readMeas, 2);

      timeLog = NOW();
      PT_SLEEP(4);

      I2C_Read(SCD30_ADDRESS, i2c_rxBuffer_scd30, 9);

      *co2 = (float)((i2c_rxBuffer_scd30[1] << 8) | i2c_rxBuffer_scd30[2]);
      *temp = (float)(-45 + 175 * ((i2c_rxBuffer_scd30[4] << 8) | i2c_rxBuffer_scd30[5]));
      *rh = (float)(100*((i2c_rxBuffer_scd30[7] << 8) | i2c_rxBuffer_scd30[8]));

      INFO("[Thread 2] Data read done.");
  }

  *pThreadDone = true;
  PT_END(pt);
}
