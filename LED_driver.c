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
#define LED_SHUTDOWN                  {0x00}
#define LED_PWM_1                     {0x01}
#define LED_PWM_2                     {0x02}
#define LED_PWM_3                     {0x03}
#define LED_PWM_4                     {0x04}
#define LED_PWM_5                     {0x05}
#define LED_PWM_6                     {0x06}
#define LED_PWM_7                     {0x07}
#define LED_PWM_8                     {0x08}
#define LED_PWM_9                     {0x09}
#define LED_PWM_10                    {0x0A}
#define LED_PWM_11                    {0x0B}
#define LED_PWM_12                    {0x0C}
#define LED_PWM_13                    {0x0D}
#define LED_PWM_14                    {0x0E}
#define LED_PWM_15                    {0x0F}
#define LED_PWM_16                    {0x10}
#define LED_PWM_17                    {0x11}
#define LED_PWM_18                    {0x12}
#define LED_EN_1_TO_6                 {0x13}
#define LED_EN_7_TO_12                {0x14}
#define LED_EN_13_TO_18               {0x15}
#define LED_APPLY                     {0x16}
#define LED_RESET                     {0x17}

// RED : 1, GREEN : 2
void LEDUpdateValue(uint8_t color)
{
  INFO("Updating LED color registers ...");

  uint8_t data[1];
  uint8_t resetRegisters[1] = LED_RESET;
  data[0] = 0x00;
  I2C_WriteCmdArgs(LED_ADDRESS, resetRegisters, data, 1, 1);

  uint8_t setSWEnable[1] = LED_SHUTDOWN;
  data[0] = 0x01; // LED OFF SOFTWARE
  I2C_WriteCmdArgs(LED_ADDRESS, setSWEnable, data, 1, 1);

  // Enable all LEDs TODO: enable only needed ones

  uint8_t enableLEDs_1_TO_6[1] = LED_EN_1_TO_6;
  uint8_t enableLEDs_7_TO_12[1] = LED_EN_7_TO_12;
  uint8_t enableLEDs_13_TO_18[1] = LED_EN_13_TO_18;
  data[0] = 0xFF;
  I2C_WriteCmdArgs(LED_ADDRESS, enableLEDs_1_TO_6, data, 1, 1);
  I2C_WriteCmdArgs(LED_ADDRESS, enableLEDs_7_TO_12, data, 1, 1);
  I2C_WriteCmdArgs(LED_ADDRESS, enableLEDs_13_TO_18, data, 1, 1);

  uint8_t driveCurrent[1];
  driveCurrent[0] = 0x32;

  uint8_t driveLed3[2] = LED_PWM_3;
  uint8_t driveLed6[2] = LED_PWM_6;
  uint8_t driveLed9[2] = LED_PWM_9;
  uint8_t driveLed12[2] = LED_PWM_12;
  uint8_t driveLed15[2] = LED_PWM_15;
  uint8_t driveLed18[2] = LED_PWM_18;

  uint8_t driveLed2[2] = LED_PWM_2;
  uint8_t driveLed5[2] = LED_PWM_5;
  uint8_t driveLed8[2] = LED_PWM_8;
  uint8_t driveLed11[2] = LED_PWM_11;
  uint8_t driveLed14[2] = LED_PWM_14;
  uint8_t driveLed17[2] = LED_PWM_17;

  switch (color) {
    case 1:
      I2C_WriteCmdArgs(LED_ADDRESS, driveLed3, driveCurrent, 1, 1);
      I2C_WriteCmdArgs(LED_ADDRESS, driveLed6, driveCurrent, 1, 1);
      I2C_WriteCmdArgs(LED_ADDRESS, driveLed9, driveCurrent, 1, 1);
      I2C_WriteCmdArgs(LED_ADDRESS, driveLed12, driveCurrent, 1, 1);
      I2C_WriteCmdArgs(LED_ADDRESS, driveLed15, driveCurrent, 1, 1);
      I2C_WriteCmdArgs(LED_ADDRESS, driveLed18, driveCurrent, 1, 1);
      break;
    case 2:
      I2C_WriteCmdArgs(LED_ADDRESS, driveLed2, driveCurrent, 1, 1);
      I2C_WriteCmdArgs(LED_ADDRESS, driveLed5, driveCurrent, 1, 1);
      I2C_WriteCmdArgs(LED_ADDRESS, driveLed8, driveCurrent, 1, 1);
      I2C_WriteCmdArgs(LED_ADDRESS, driveLed11, driveCurrent, 1, 1);
      I2C_WriteCmdArgs(LED_ADDRESS, driveLed14, driveCurrent, 1, 1);
      I2C_WriteCmdArgs(LED_ADDRESS, driveLed17, driveCurrent, 1, 1);
      break;
  }

  // update all registers
  uint8_t updateRegisters[1] = LED_APPLY;
  data[0] = 0x00;
  I2C_WriteCmdArgs(LED_ADDRESS, updateRegisters, data, 1, 1);

}
