#include "sl_component_catalog.h"
#include "sl_system_init.h"
#include "app.h"
//#include "ustimer.h"
#include "sl_system_process_action.h"
#include "em_gpio.h"

void initGPIO(void)
{
  // Buzzer Enable = PD04
  GPIO_PinModeSet(gpioPortD, 4, gpioModePushPull, 1);
  // Led Driver Enable = PD03
  GPIO_PinModeSet(gpioPortD, 3, gpioModePushPull, 1);

  GPIO_PinOutClear(gpioPortD, 4);
  GPIO_PinOutClear(gpioPortD, 3);
}

int main(void)
{
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  // sl_system_init includes sl_ot_create_instance() and sl_ot_cli_init()
  sl_system_init();
  otCliOutputFormat("System initialized, Thread enabled");

  initGPIO();
  //USTIMER_init(); // TODO: remove?
  app_init();

  while (1) {
    // Do not remove this call: Silicon Labs components process action routine
    // must be called from the super loop.
    sl_system_process_action();

    // Application process.
    app_process_action();

    // TODO : go to sleep

  }
  // Clean-up when exiting the application.
  app_exit();
}
