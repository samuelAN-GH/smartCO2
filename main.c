#include "sl_component_catalog.h"
#include "sl_system_init.h"
#include "app.h"
#include "ustimer.h"
#include "sl_system_process_action.h"
#include <openthread/cli.h>
#include "em_gpio.h"
#include "i2c_.h"
#include <openthread-core-config.h>
#include <openthread/config.h>
#include <openthread/thread.h>
#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>

void initGPIO(void)
{
  // Buzzer Enable = PD04
  GPIO_PinModeSet(gpioPortD, 4, gpioModePushPull, 1);
  // Led Driver Enable = PD03
  GPIO_PinModeSet(gpioPortD, 3, gpioModePushPull, 1);

  GPIO_PinOutClear(gpioPortD, 4);
  GPIO_PinOutSet(gpioPortD, 3);
}

int main(void)
{
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  // sl_system_init includes sl_ot_create_instance() and sl_ot_cli_init()
  sl_system_init();

  initGPIO();
  USTIMER_Init(); // TODO: remove?

  AppData *app = app_init(otGetInstance());
  initI2C();

  otCliOutputFormat("System initialized, Thread enabled \r\n");
  otCliOutputFormat("Main loop starting ...");

  while (1) {

    sl_system_process_action(); //TODO : remove?

    // Application process.
    app_process_action();


  }
  // Clean-up when exiting the application.
  app_exit();
}
