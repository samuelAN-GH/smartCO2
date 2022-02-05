#ifndef SCD41_H_
#define SCD41_H_

#include <i2c_.h>
#include "app.h"
#include <openthread/platform/alarm-milli.h>
#include "pt/pt.h"
#include <stdio.h>
#include "coap_.h"
#include "dns.h"
#include "config_.h"

PT_THREAD(SCD41UpdateValueThread(struct pt *pt, bool *pThreadDone3, float *co2, float *temp, float *rh));

void SCD41Init();

#endif /* SCD41_H_ */
