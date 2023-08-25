#include "sl_component_catalog.h"
#include "sl_system_init.h"
#include "sl_system_process_action.h"
#include <sl_power_manager.h>
#include <sl_sleeptimer.h>

#include <openthread/cli.h>
#include <openthread-core-config.h>
#include <openthread/config.h>
#include <openthread/thread.h>
#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>
#include <openthread-system.h>

#include "em_gpio.h"
#include "app.h"
#include "i2c_.h"
#include "log.h"

#define APP_CYCLE 10 // Main app period cycle [s]

static sl_sleeptimer_timer_handle_t timer;  // Main app sequencing timer
static bool run = true;                     // allow AppProcess to run

void initGPIO(void)
{
  // Unused GPIOs
  GPIO_PinModeSet(gpioPortA, 1, gpioModeDisabled, 1);
  GPIO_PinModeSet(gpioPortA, 2, gpioModeDisabled, 1);
  GPIO_PinModeSet(gpioPortA, 3, gpioModeDisabled, 1);
  GPIO_PinModeSet(gpioPortA, 4, gpioModeDisabled, 1);
  GPIO_PinModeSet(gpioPortA, 5, gpioModeDisabled, 1);
  GPIO_PinModeSet(gpioPortA, 6, gpioModeDisabled, 1);

  GPIO_PinModeSet(gpioPortD, 0, gpioModeDisabled, 1);
  GPIO_PinModeSet(gpioPortD, 1, gpioModeDisabled, 1);
  GPIO_PinModeSet(gpioPortD, 2, gpioModeDisabled, 1);

  GPIO_PinModeSet(gpioPortC, 0, gpioModeDisabled, 1);
  GPIO_PinModeSet(gpioPortC, 1, gpioModeDisabled, 1);
  GPIO_PinModeSet(gpioPortC, 2, gpioModeDisabled, 1);
  GPIO_PinModeSet(gpioPortC, 3, gpioModeDisabled, 1);
  GPIO_PinModeSet(gpioPortC, 4, gpioModeDisabled, 1);
  GPIO_PinModeSet(gpioPortC, 5, gpioModeDisabled, 1);

  // Buzzer Enable = PD04
  GPIO_PinModeSet(gpioPortD, 4, gpioModePushPull, 1);
  // Led Driver Enable = PD03
  GPIO_PinModeSet(gpioPortD, 3, gpioModePushPull, 1);

  GPIO_PinOutClear(gpioPortD, 4);
  GPIO_PinOutClear(gpioPortD, 3);
}

void timer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  run = true;
}

int main(void)
{
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  // sl_system_init includes sl_ot_create_instance() and sl_ot_cli_init()
  sl_system_init();

  initGPIO();
  initI2C();
  sl_sleeptimer_init();
  sl_power_manager_init();

  app_init(otGetInstance());

  otCliOutputFormat("System initialized, Thread enabled \r\n");
  otCliOutputFormat("Main loop starting ...");

  uint32_t tickNumbers = 32768*APP_CYCLE;
  sl_sleeptimer_start_timer(&timer, tickNumbers, timer_callback, (void *)NULL, 0, 0);

  while (1) {
    sl_system_process_action();

    // TaskletsProcess & SysProcess equivalent to old "thread_step"
    // Thread nwk synchronization (OT SYNC)
    otTaskletsProcess(otGetInstance());
    otSysProcessDrivers(otGetInstance());

    // Application process.
    if (run) {
        app_process_action(&run);
        if (!run) {
            sl_sleeptimer_restart_timer(&timer, tickNumbers, timer_callback, (void *)NULL, 0, 0);
        }
    }
  }
  // Clean-up when exiting the application.
  app_exit();
}
