#include "dns.h"
#include "log.h"

#include <string.h>
#include <openthread/srp_client.h>
#include <openthread/dns.h>
#include <openthread/dns_client.h>

/*------------------------------------------------------------------------------------------------+
 |                                     Private data structure                                     |
 +------------------------------------------------------------------------------------------------*/

#define SERVICE_MAX_LEN 128

struct MDnsClient {
    otInstance *instance;               //!< The openthread instance
    bool ready;                         //!< Whether the client is ready to search.
    bool busy;                          //!< Whether the client is busy sending data.
    char service_name[SERVICE_MAX_LEN]; //!< Contains the name of the service to look for.
    bool *done;                         //!< Whether the sending process is over (for context)
    otError *error;                     //!< Contains the error (for context)
    otIp6Address *address;              //!< The address if one is found (for context)
};

static MDnsClient _client;

/*------------------------------------------------------------------------------------------------+
 |                                       Callback functions                                       |
 +------------------------------------------------------------------------------------------------*/

static void srp_client_autostart_callback(const otSockAddr *sock, void *context)
{
    MDnsClient *client = context;
    otError error = OT_ERROR_NONE;

    if (! sock) {
        ERROR("Srp client has stopped");
        client->ready = false;
    }
    INFO("Srp client has started");
    client->ready = true;
}

void dns_result_callback(otError error, const otDnsBrowseResponse *response, void *context)
{
    MDnsClient *client = context;

    char *hostname;
    otIp6Address addr;

    char name[OT_DNS_MAX_NAME_SIZE];
    char label[OT_DNS_MAX_LABEL_SIZE];
    char ip[64];
    uint8_t txt[255];
    otDnsServiceInfo service_info;


    if (error != OT_ERROR_NONE) {
        ERROR_F("MDns callback");
        goto cleanup;
    }

    error = otDnsBrowseResponseGetServiceName(response, name, sizeof(name));
    if (error != OT_ERROR_NONE) {
        ERROR_F("otDnsBrowseResponseGetServiceName");
        goto cleanup;
    }

    error = otDnsBrowseResponseGetServiceInstance(response, 0, label, OT_DNS_MAX_LABEL_SIZE);
    if (error != OT_ERROR_NONE) {
        ERROR_F("otDnsBrowseResponseGetServiceInstance");
        goto cleanup;
    }

    INFO("Found service: %s", name);

    service_info.mHostNameBuffer = name;
    service_info.mHostNameBufferSize = sizeof(name);
    service_info.mTxtData = txt;
    service_info.mTxtDataSize = 255;

    error = otDnsBrowseResponseGetServiceInfo(response, label, &service_info);
    if (error != OT_ERROR_NONE) {
        ERROR_F("otDnsBrowseResponseGetServiceInfo");
        goto cleanup;
    }

    otDnsTxtEntry entry;
    otDnsTxtEntryIterator iterator;

    otDnsInitTxtEntryIterator(&iterator, service_info.mTxtData, service_info.mTxtDataSize);

    while (otDnsGetNextTxtEntry(&iterator, &entry) == OT_ERROR_NONE) {
        if (entry.mKey == NULL) {
            continue;
        }

        if (strcmp(entry.mKey, "addr") == 0) {
            strncpy(ip, (char *)entry.mValue, entry.mValueLength);
            error = otIp6AddressFromString(ip, client->address);
            if (error != OT_ERROR_NONE) {
                ERROR_F("otIp6AddressFromString");
                goto cleanup;
            }
            INFO("Found addr: %s", ip);
            break;
        }
    }

cleanup:
    *client->done = true;
    *client->error = error;
    client->busy = false;
}

/*------------------------------------------------------------------------------------------------+
 |                                       API implementation                                       |
 +------------------------------------------------------------------------------------------------*/

MDnsClient *mdns_init(otInstance *instance, const char *service_name)
{
    _client.instance = instance;
    _client.ready = false;
    _client.busy = false;

    if (strlen(service_name) >= SERVICE_MAX_LEN) {
        ERROR("Service name too long");
        return NULL;
    }
    strncpy((char *)&_client.service_name, service_name, SERVICE_MAX_LEN);

    _client.done = NULL;
    _client.error = NULL;
    _client.address = NULL;

    otSrpClientEnableAutoStartMode(instance, srp_client_autostart_callback, &_client);

    return &_client;
}

bool mdns_is_ready(MDnsClient *client)
{
    return client->ready;
}

otError mdns_schedule_search(MDnsClient *client, otIp6Address *address, bool *done, otError *error_out)
{
    otError error;

    // 0. Sanity check
    if (client->busy || ! client->ready) {
        return OT_ERROR_BUSY;
    }

    // 1. Beginnig the search
    *done = false;
    client->busy = true;

    // 2. Recording where to store the search results.
    client->address = address;
    client->done = done;
    client->error = error_out;

    // 3. Launching the search
    INFO("Launching the MDNS search...");
    error = otDnsClientBrowse(client->instance, client->service_name, dns_result_callback, client, NULL);
    if (error != OT_ERROR_NONE) {
        ERROR_F("otDnsClientBrowse");
        client->busy = false;
        *error_out = OT_ERROR_ABORT;
        *done = true;
        return error;
    }

    return OT_ERROR_NONE;
}
