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

#include "sl_component_catalog.h"


#ifndef OPENTHREAD_ENABLE_COVERAGE
#define OPENTHREAD_ENABLE_COVERAGE 0
#endif

typedef enum AppState
{
    MDNS_READY,
    MDNS_SEARCHING,
    MDNS_WAIT_ANSWER,
    COAP_INIT,
    DATA_READ,
    COAP_POST,
    COAP_WAIT_ACK,
    DELAY1S,
    DELAY5S,
    ERROR
} AppState;

struct AppData {
    struct pt pt;
    otInstance *instance;
    CoapClient *coap;
    MDnsClient *mdns;
    AppState state;
    AppState next_state;
    bool done;
    bool connected;
    uint64_t then;
    otIp6Address address;
    otError error;
    uint16_t dummy;
    char message[APP_MESSAGE_MAX_LEN];
    bool txInProgress;
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

/**************************************************************************//**
 * Application Init, returns AppData
 *****************************************************************************/

AppData *app_init(otInstance *instance)
{

    setNetworkConfiguration(); // Load Thread network conf and reset thread
    otError error;
    AppData *app = &_app;

    app->instance = instance;
    app->coap = NULL;
    app->mdns = NULL;
    app->done = true;
    app->then = otPlatAlarmMilliGetNow();
    app->connected = false;

    // PT_INIT(&app->pt);

    // TODO: remove
    app->dummy = 400;

//    error = otSetStateChangedCallback(instance, handleNetifStateChanged, app);
//    if (error != OT_ERROR_NONE) {
//        ERROR_F("otSetStateChangedCallback");
//        return NULL;
//    }

    return app;

    GPIO_PinOutSet(gpioPortD, 4);
    USTIMER_Delay(200000);
    GPIO_PinOutClear(gpioPortD, 4);
}


/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
    otTaskletsProcess(sInstance);
    otSysProcessDrivers(sInstance);
}

/**************************************************************************//**
 * Application Exit.
 *****************************************************************************/
void app_exit(void)
{
    otInstanceFinalize(sInstance);

}
