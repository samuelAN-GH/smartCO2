#include "config_.h"
#include "coap_.h"
#include <openthread-core-config.h>
#include <openthread/config.h>

#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <openthread/coap.h>
#include <openthread/ip6.h>

#include <string.h>
#include <stdio.h>

#include "log.h"

#include "ustimer.h"
#include "em_gpio.h"

/*------------------------------------------------------------------------------------------------+
 |                                     Private data structure                                     |
 +------------------------------------------------------------------------------------------------*/

/**
 * @brief      The maximum number of character a URI can hold.
 */
#define URI_MAX_LEN 128

/**
 * @brief      The maximum number of bytes a message can hold.
 */
#define MSG_MAX_LEN 1024

struct CoapClient {
    otInstance *instance;  //!< The openthread instance
    otMessageInfo target;  //!< The ipv6 and port on the target.
    otMessage *message;    //!< A pointer to the current message to send.
    char uri[URI_MAX_LEN]; //!< The target's uri.
    bool busy;             //!< Whether the client is busy sending data.
    bool *done;            //!< Whether the sending process is over (for context)
    otError *error;        //!< Contains an error if one occurs during sending (for context)
};

static CoapClient _client;

/*------------------------------------------------------------------------------------------------+
 |                                       Callback functions                                       |
 +------------------------------------------------------------------------------------------------*/

static void coap_request_callback(void *context,
                                  otMessage *message,
                                  const otMessageInfo *message_info,
                                  otError error)
{
    CoapClient *client = context;

    if (error == OT_ERROR_NONE) {
        INFO("Coap message sent: %s", otCoapMessageCodeToString(message));
//        GPIO_PinOutSet(gpioPortD, 4);
//        USTIMER_Delay(4000000);
//        GPIO_PinOutClear(gpioPortD, 4);
    }
    else {
        ERROR("Failed to send coap message: %s", otCoapMessageCodeToString(message));

    }

    *client->done = true;
    *client->error = error;
    client->busy = false;

    if ((error != OT_ERROR_NONE) && message) {
        otMessageFree(message);
    }
}

/*------------------------------------------------------------------------------------------------+
 |                                       API implementation                                       |
 +------------------------------------------------------------------------------------------------*/

CoapClient *coap_init(otInstance *instance, otIp6Address *target)
{
    _client.instance = instance;
    memcpy(&_client.target.mPeerAddr, target, sizeof(otIp6Address));
    _client.target.mPeerPort = APP_SERVICE_PORT;
    _client.busy = false;
    _client.message = NULL;

    int wr = snprintf(_client.uri, URI_MAX_LEN, APP_URI_TEMPLATE, APP_TOKEN);
    if (wr < 0 || wr == URI_MAX_LEN) {
        return NULL;
    }

    otError error = otCoapStart(instance, APP_SERVICE_PORT);
    if (error != OT_ERROR_NONE) {
        ERROR_F("otCoapStart");
        return NULL;
    }

    INFO("Coap enabled");

    return &_client;
}

bool coap_update_target(CoapClient *client, otIp6Address *target)
{
    if (otIp6IsAddressEqual(target, &client->target.mPeerAddr)) {
        return false;
    }

    memcpy(&client->target.mPeerAddr, target, sizeof(otIp6Address));

    INFO("Coap target updated");
    return true;
}

otError coap_schedule_post(CoapClient *client,
                           char *message,
                           uint16_t message_size,
                           bool *done,
                           otError *error_out)
{
    bool success;
    otError error;



    // 0. Sanity check
    if (client->busy) {
        return OT_ERROR_BUSY;

    }
    if (message_size >= MSG_MAX_LEN) {
        ERROR("Message to long!");
//        GPIO_PinOutSet(gpioPortD, 4);
//        USTIMER_Delay(2000000);
//        GPIO_PinOutClear(gpioPortD, 4);
        return OT_ERROR_FAILED;
    }

    // 1. Beginning the sending process.
    *done = false;
    client->busy = true;
    client->done = done;
    client->error = error_out;



    // 2. Allocate a new outgoing message.
    client->message = otCoapNewMessage(client->instance, NULL);
    if (! client->message) {
        error = OT_ERROR_NO_BUFS;
        goto cleanup;
    }



    // 3. Set it as confirmable, POST and with the default token length.
    otCoapMessageInit(client->message, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_POST);
    otCoapMessageGenerateToken(client->message, OT_COAP_DEFAULT_TOKEN_LENGTH);

    // 4. Add the URI to the message.
    error = otCoapMessageAppendUriPathOptions(client->message, client->uri);
    if (error != OT_ERROR_NONE) {
        ERROR("Couldn't set the URI to the message");
        goto cleanup;
    }

    // 5. Add the payload to the message
    error = otCoapMessageSetPayloadMarker(client->message);
    if (error != OT_ERROR_NONE) {
        ERROR("Couldn't set the payload marker");
        goto cleanup;
    }
    if (message_size >= MSG_MAX_LEN) {
    }
    error = otMessageAppend(client->message, message, message_size);
    if (error != OT_ERROR_NONE) {
        ERROR_F("otMessageAppend");
        goto cleanup;
    }

    // 6. Schedule the message for sending.

    error = otCoapSendRequest(
        client->instance, client->message, &client->target, coap_request_callback, client);

    if (error != OT_ERROR_NONE) {
        ERROR_F("otCoapSendRequest");
        goto cleanup;
    }



    return OT_ERROR_NONE;

cleanup:

    *done = true;
    *error_out = OT_ERROR_ABORT;
    otMessageFree(client->message);
    return error;
}
