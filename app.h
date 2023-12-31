#ifndef APP_H
#define APP_H

#include "pt/pt.h"
#include <openthread/thread.h>

typedef struct AppData AppData;

otInstance *otGetInstance(void);

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
AppData *app_init(otInstance *instance);

/**************************************************************************//**
 * Application Exit.
 *****************************************************************************/
void app_exit(void);

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(bool *run);

PT_THREAD(nodeConnectThread(struct pt *pt));
PT_THREAD(coap_sending_process(AppData *app, bool *run));

#endif
