#include <i2c_.h>
#include "LED_driver.h"
#include "app.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "log.h"

#define LED_ADDRESS 0x54      // 7 bits I2C address

// SCD41 commands list
#define LED_SHUTDOWN                  {0x00, 0x00}
#define LED_PWM_1                     {0x00, 0x01}
#define LED_PWM_2                     {0x00, 0x02}
#define LED_PWM_3                     {0x00, 0x03}
#define LED_PWM_4                     {0x00, 0x04}
#define LED_PWM_5                     {0x00, 0x05}
#define LED_PWM_6                     {0x00, 0x06}
#define LED_PWM_7                     {0x00, 0x07}
#define LED_PWM_8                     {0x00, 0x08}
#define LED_PWM_9                     {0x00, 0x09}
#define LED_PWM_10                    {0x00, 0x10}
#define LED_PWM_11                    {0x00, 0x11}
#define LED_PWM_12                    {0x00, 0x12}
#define LED_EN_1_TO_6                 {0x00, 0x13}
#define LED_EN_2_TO_12                {0x00, 0x14}
#define LED_EN_13_TO_18               {0x00, 0x15}
#define LED_APPLY                     {0x00, 0x16}
#define LED_RESET                     {0x00, 0x17}

void LEDUpdateValueThread(uint8_t *color)
{
  INFO("Updating LED color registers ...");

  uint8_t setSWEnable[2] = LED_SHUTDOWN;
  uint8_t data[1];
  data[0] = 1;
  I2C_WriteCmdArgs(LED_ADDRESS, setSWEnable, data);
}
