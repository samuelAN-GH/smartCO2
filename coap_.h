#ifndef THINGSBOARD_APP_COAP_H
#define THINGSBOARD_APP_COAP_H

/**
 * @file Single target coap client.
 */

#include "app.h"

#include <openthread/instance.h>
#include <openthread/ip6.h>

/**
 * @brief      Opaque data structure for the coap client.
 */
typedef struct CoapClient CoapClient;

/**
 * @brief      Initialize the coap client.
 *
 * @param[in]  instance  The openthread instance to use for the client.
 * @param[in]  target    The target to send data to.
 *
 * @return     The initialized coap client, or NULL if an error occurred.
 *
 * @remark     Only one of these can be instantiated. The returned instance is static.
 */
CoapClient *coap_init(otInstance *instance, otIp6Address *target);

/**
 * @brief     Update the target address of the client.
 *
 * @param[in]     client The client to update the address of.
 * @param[in]     target The new target.
 *
 * @return        True if the target was updated, false otherwise.
 */
bool coap_update_target(CoapClient *client, otIp6Address *target);

/**
 * @brief      Schedule to send (POST) message to the client's target.
 *
 * @param[in]  client        The client to send the message with.
 * @param[in]  message       The message to send.
 * @param[in]  message_size  The size of the message to send.
 * @param[in]  done          A boolean the function uses to signal it has finished sending the
 *                           message. The 'done' variable will be set to 'false' when the message
 *                           has been schedule and to 'true' when it has been confirmed to have
 *                           been sent.
 * @param[in]  error         OT_MESSAGE_NONE if the message was sent successfully, the corresponding
 *                           error otherwise.
 *
 * @return     OT_MESSAGE_NONE if the message was properly scheduled, the corresponding error
 *             otherwise.
 */
otError coap_schedule_post(CoapClient *client,
                           char *message,
                           uint16_t message_size,
                           bool *done,
                           otError *error);

#endif
