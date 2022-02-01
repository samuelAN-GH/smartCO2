#include <assert.h>
#include "coap_.h"
#include "config_.h"
#include <openthread-core-config.h>
#include <openthread/config.h>
#include "ustimer.h"

#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <openthread/dataset_ftd.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "em_gpio.h"

#include "openthread-system.h"
#include "app.h"
#include "dns.h"
#include "pt/pt.h"
#include "i2c.h"
#include "log.h"

#include "sl_component_catalog.h"
#include <openthread/platform/alarm-milli.h>

#ifndef OPENTHREAD_ENABLE_COVERAGE
#define OPENTHREAD_ENABLE_COVERAGE 0
#endif

#define NOW()                 otPlatAlarmMilliGetNow()
#define PT_SLEEP(x)           PT_WAIT_UNTIL(pt, (NOW() - _app.then) >= (x))

#define PROTOTHREADS_NUMBER 1           // Number of protothreads in app_process_action

struct AppData {
    struct pt ptSCD30;                  // SCD30UpdateValueThread pointer
    struct pt ptConnect;                // nodeConnectThread pointer
    struct pt ptSCD41;                  // SCD41UpdateValueThread pointer
    struct pt ptVbat;                   // VbatUpdateValueThread pointer
    otInstance *sInstance;              // OT instance
    CoapClient *coap;                   // Coap client instance
    MDnsClient *mdns;                   // MDNS client instance
    bool done;                          // MDNS client connected to thingsboard
    bool connected;                     // Current OT instance connected
    uint64_t then;
    otIp6Address address;
    otError error;
    float co2;
    float temp;
    float rh;
    uint8_t vbat;
    char message[APP_MESSAGE_MAX_LEN];
    bool txInProgress;                  // I2C tx in progress flag
    uint8_t appThreadsSuccess;          // Number of successfuly finished Threads
};

static AppData _app;

extern void otAppCliInit(otInstance *aInstance);

#if OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE
void *otPlatCAlloc(size_t aNum, size_t aSize)
{
    return calloc(aNum, aSize);
}

void otPlatFree(void *aPtr)
{
    free(aPtr);
}
#endif

static otInstance *    sInstance       = NULL;

otInstance *otGetInstance(void)
{
    return sInstance;
}

#if OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_APP
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    OT_UNUSED_VARIABLE(aLogLevel);
    OT_UNUSED_VARIABLE(aLogRegion);
    OT_UNUSED_VARIABLE(aFormat);

    va_list ap;
    va_start(ap, aFormat);
    otCliPlatLogv(aLogLevel, aLogRegion, aFormat, ap);
    va_end(ap);
}
#endif

void sl_ot_create_instance(void)
{
    sInstance = otInstanceInitSingle();
    assert(sInstance);
}

void sl_ot_cli_init(void)
{
    otAppCliInit(sInstance);
}

static int build_message(AppData *app, float val)
{
    int wr = snprintf(app->message, APP_MESSAGE_MAX_LEN, APP_MESSAGE_TEMPLATE, val);
    if (wr < 0 || wr >= APP_MESSAGE_MAX_LEN) {
        return -1;
    }

    return wr;
}

/*
===============================================
Thread Nwk configuration & state change handler
===============================================
*/

// Callback for thread state change & update app->connected state
static void handleNetifStateChanged(uint32_t flags, void *context)
{
    AppData *app = context;

    if ((flags & OT_CHANGED_THREAD_ROLE) != 0) {
        otDeviceRole changedRole = otThreadGetDeviceRole(app->sInstance);

        INFO("ROLE: %d", changedRole);

        switch (changedRole) {
        case OT_DEVICE_ROLE_LEADER:
        case OT_DEVICE_ROLE_ROUTER:
        case OT_DEVICE_ROLE_CHILD:
            if (! app->connected) {
                INFO("Connected!");
            }
            app->connected = true;
            break;

        case OT_DEVICE_ROLE_DETACHED:
        case OT_DEVICE_ROLE_DISABLED:
            // TODO: better handle disconnections
            INFO("Disconnected!");
            app->connected = false;
            break;
        }
    }
}

void setNetworkConfiguration(void)
{
    static char          aNetworkName[] = APP_NETWORK_NAME;
    otError              error;
    otOperationalDataset aDataset;

    memset(&aDataset, 0, sizeof(otOperationalDataset));

    aDataset.mActiveTimestamp                      = 1;
    aDataset.mComponents.mIsActiveTimestampPresent = true;

    /* Set Channel */
    aDataset.mChannel                      = APP_NETWORK_CHANNEL;
    aDataset.mComponents.mIsChannelPresent = true;

    /* Set Pan ID */
    aDataset.mPanId                      = (otPanId)APP_NETWORK_PANID;
    aDataset.mComponents.mIsPanIdPresent = true;

    /* Set Extended Pan ID */
    uint8_t extPanId[OT_EXT_PAN_ID_SIZE] = APP_NETWORK_EPANID;
    memcpy(aDataset.mExtendedPanId.m8, extPanId, sizeof(aDataset.mExtendedPanId));
    aDataset.mComponents.mIsExtendedPanIdPresent = true;

    /* Set network key */
    uint8_t key[OT_NETWORK_KEY_SIZE] = APP_NETWORK_KEY;
    memcpy(aDataset.mNetworkKey.m8, key, sizeof(aDataset.mNetworkKey));
    aDataset.mComponents.mIsNetworkKeyPresent = true;

    /* Set Network Name */
    size_t length = strlen(aNetworkName);
    assert(length <= OT_NETWORK_NAME_MAX_SIZE);
    memcpy(aDataset.mNetworkName.m8, aNetworkName, length);
    aDataset.mComponents.mIsNetworkNamePresent = true;

    /* Set the Active Operational Dataset to this dataset */
    error = otDatasetSetActive(otGetInstance(), &aDataset);

    if (error != OT_ERROR_NONE)
    {
        otCliOutputFormat("Error applying thread dataset (Nwk conf): %d, %s\r\n", error, otThreadErrorToString(error));
        return;
    }

    otLinkModeConfig config;

    config.mRxOnWhenIdle = true;
    config.mDeviceType   = 0; //MTD
    config.mNetworkData  = 0;
    error = otThreadSetLinkMode(otGetInstance(), config);

    assert(otIp6SetEnabled(sInstance, true) == OT_ERROR_NONE);
    assert(otThreadSetEnabled(sInstance, true) == OT_ERROR_NONE);

    GPIO_PinOutSet(gpioPortD, 4);
    USTIMER_Delay(200000);
    GPIO_PinOutClear(gpioPortD, 4);
}

/*
=======================
Application Init
out : AppData structure
=======================
*/

AppData *app_init(otInstance *instance)
{
    setNetworkConfiguration(); // Load Thread network conf and reset thread
    otError error;
    AppData *app = &_app;

    app->sInstance = instance;
    app->coap = NULL;
    app->mdns = NULL;
    app->done = true;
    app->then = otPlatAlarmMilliGetNow();
    app->connected = false;

    PT_INIT(&(_app).ptConnect);
    PT_INIT(&(_app).ptSCD30);
    PT_INIT(&(_app).ptSCD41);
    PT_INIT(&(_app).ptVbat);

    app->co2 = 0.0;
    app->rh = 0.0;
    app->temp = 0.0;
    app->vbat = 0;

    app->appThreadsSuccess = 0;

    // Associate a callback in case of Thread state change (disabled, child, ...)
    error = otSetStateChangedCallback(instance, handleNetifStateChanged, app);
    if (error != OT_ERROR_NONE) {
        ERROR_F("otSetStateChangedCallback");
        return NULL;
    }

    return app;

    GPIO_PinOutSet(gpioPortD, 4);
    USTIMER_Delay(200000);
    GPIO_PinOutClear(gpioPortD, 4);
}

/*
==============================================
Application Process / Called by main superloop
==============================================
*/

void app_process_action(void)
{

    // TaskletsProcess & SysProcess equivalent to old "thread_step"
    // Thread nwk synchronization (OT SYNC)
    otTaskletsProcess(sInstance);
    otSysProcessDrivers(sInstance);

    uint8_t th1 = nodeConnectThread(&(_app).ptConnect);         // Node thread connection Thread
    uint8_t th2 = 3; // SCD30UpdateValueThread(&(_app).ptSCD30);    // SCD30 measurement Thread (if connected)
    uint8_t th3 = 3; // SCD41UpdateValueThread(&(_app).ptSCD41);    // SCD41 measurement Thread
    uint8_t th4 = 3; //VbatUpdateValueThread(&(_app).ptVbat);      // Battery voltage measurement Thread

    // Check that all threads are finished (return 3)
    if(th1 == 3 && th2 == 3 && th3 == 3 && th4 == 3) {

      INFO("All app Threads finished, sending coap message...");
      _app.appThreadsSuccess = 0;

      //build message()
      //send message()

      INFO("CoAP message sent, going in sleep mode...");

      //Sleep (x min)

      GPIO_PinOutSet(gpioPortD, 4);
      USTIMER_Delay(200000);
      GPIO_PinOutClear(gpioPortD, 4);

    }
}

PT_THREAD(nodeConnectThread(struct pt *pt))
{
    if(_app.appThreadsSuccess == PROTOTHREADS_NUMBER) {
        PT_EXIT(pt); // All threads are flaged as finished
    }

    INFO("[Thread 1] Node Connection...");

    AppData *app = &_app; //pointer to our static app structure
    static otError error = OT_ERROR_NONE;

    PT_BEGIN(pt);

    // 0. Wait for the node to be connected.
    while (true) {
        if (_app.connected) {
            break;
        }

        INFO("[Thread 1] Waiting for connection...");
        PT_SLEEP(1000);
        _app.then = NOW();
    }

    // 1. initialize the mdns client.
    if(!_app.mdns) {
        INFO("[Thread 1] MDNS client init");
        _app.mdns = mdns_init(_app.sInstance, APP_SERVICE_NAME);
        if (! _app.mdns) {
            ERROR("[Thread 1] Failed to initialize mdns client");
            PT_SLEEP(1000);
            PT_RESTART(pt);
        }

      // 2. Wait for the mdns client to be ready.
      PT_WAIT_UNTIL(pt, mdns_is_ready(_app.mdns));
    }
    INFO("[Thread 1] MDNS client ready");

    // 3. Search for the thinsgboard service.
    if (!_app.done) {
        INFO("[Thread 1] MDNS client searching...");
        _app.done = false;
        _app.error = OT_ERROR_NONE;
        _app.then = NOW();
        error = mdns_schedule_search(_app.mdns, &(_app.address), &(_app.done), &(_app.error));

        // 4. Waiting for the mdns answer.
        PT_WAIT_UNTIL(pt, _app.done || (NOW() - _app.then) >= 15000);

        // 4.1 MDNS Timeout
        if (! _app.done) {
            ERROR("[Thread 1] MDNS browse: timed out");
            PT_RESTART(pt);
        }

        // 4.2 MDNS Error
        if (_app.error != OT_ERROR_NONE) {
            error = _app.error;
            ERROR_F("[Thread 1] MDNS browse");
            PT_RESTART(pt);
        }
    }
    INFO("[Thread 1] MDNS client connected to thingsboard");

    if (!_app.coap) {
        // 5. Initialize the coap client
        INFO("[Thread 1] Coap client init");
        _app.coap = coap_init(_app.sInstance, &(_app.address));
        if (! _app.coap) {
            ERROR("[Thread 1] Failed to initialize the coap client");
            PT_RESTART(pt);
        }
    }
    INFO("[Thread 1] CoAP initialized");

    _app.appThreadsSuccess++;

    // Current thread done, waiting for others threads to be done to end the current thread
    PT_WAIT_UNTIL(pt, _app.appThreadsSuccess == PROTOTHREADS_NUMBER);

    PT_END(pt);
}

// 6. Data reading/sending loop.
//while (true) {
//
//    INFO("Entering main thread data reading/sending loop ");
//
//    // Data collecting goes here.
////        _app->co2 = SCD41_get_co2();
////        _app->temp = SCD41_get_temp();
////        _app->rh = SCD41_get_rh();
//
//    message_size = build_message(&_app, _app->co2);
//
//    // Sending data to thingsboard
//
//    INFO("Sending coap message");
//    _app->done = false;
//    _app->error = OT_ERROR_NONE;
//    _app->then = NOW();
//
//    error = coap_schedule_post(_app->coap, _app->message, message_size, &_app->done, &_app->error);
//    if (error != OT_ERROR_NONE) {
//        ERROR_F("coap_schedule_post");
//        PT_SLEEP(1000);
//        continue;
//    }
//
//    // Waiting for the coap ack.
//    PT_WAIT_UNTIL(&_app->pt, _app->done || (NOW() - _app->then) >= 3000);
//
//    // Coap time out
//    if (! _app->done) {
//        ERROR("Coap: timed out");
//        continue;
//    }
//
//    // Coap error
//    if (_app->error != OT_ERROR_NONE) {
//        ERROR("Coap error");
//        PT_SLEEP(1000);
//        PT_RESTART(&_app->pt);
//    }
//
//    // No problem: sleep for 5s until next reading period.
//    PT_SLEEP(1000);
//}

/*
======================================
Application Exit - OT instance closing
======================================
*/

void app_exit(void)
{
    otInstanceFinalize(sInstance);

}
