#ifndef SCD30_H_
#define SCD30_H_

#include <i2c_.h>
#include "app.h"
#include <openthread/platform/alarm-milli.h>
#include "pt/pt.h"
#include <stdio.h>
#include "coap_.h"
#include "dns.h"
#include "config_.h"

PT_THREAD(SCD30UpdateValueThread(struct pt *pt, bool *pThreadDone, float *co2, float *temp, float *rh));

#endif /* SCD30_H_ */
