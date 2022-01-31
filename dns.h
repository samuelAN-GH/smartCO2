#ifndef THINGSBOARD_APP_DNS_H
#define THINGSBOARD_APP_DNS_H

/**
 * @file Service discovery client.
 */

#include <openthread/instance.h>
#include <openthread/ip6.h>

/**
 * @brief      Opaque data structure for the mdns client.
 */
typedef struct MDnsClient MDnsClient;

/**
 * @brief      Initialize the mdns client.
 *
 * @param[in]  instance  The openthread instance to use for the client.
 * @param[in]  target    The service to look for.
 *
 * @return     The initialized mdns client, or NULL if an error occurred.
 *
 * @remark     Only one of these can be instantiated. The returned instance is static.
 */
MDnsClient *mdns_init(otInstance *instance, const char *service_name);

/**
 * @brief      Determine whether the client is ready to look for the service.
 *
 * @param      client    The client to check the readiness of.
 *
 * @return     Whether the client is ready or not.
 */
bool mdns_is_ready(MDnsClient *client);

/**
 * @brief      Schedule to send (POST) message to the client's target.
 *
 * @param[in]  client        The client to search with.
 * @param[in]  address       A pointer to copy the resulting found address to
 * @param[in]  done          A boolean the function uses to signal it has finished finished searching
 *                           message.
 * @param[in]  error         OT_MESSAGE_NONE if the search was successful, the corresponding
 *                           error otherwise.
 *
 * @return     OT_MESSAGE_NONE if the search was properly scheduled, the corresponding error
 *             otherwise.
 */
otError mdns_schedule_search(MDnsClient *client, otIp6Address *address, bool *done, otError *error);


#endif
