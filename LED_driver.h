#ifndef LED_DRIVER_H_
#define LED_DRIVER_H_

#include <i2c_.h>
#include "LED_driver.h"
#include "app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "log.h"

void LEDUpdateValue(uint8_t color);

#endif /* LED_DRIVER_H_ */
