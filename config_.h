#ifndef THINGSBOARD_CONFIG_H
#define THINGSBOARD_CONFIG_H

#include <stdint.h>
#include <openthread/dataset_ftd.h>
#include <openthread/coap.h>

// Thingsboard config

#define APP_MESSAGE_MAX_LEN 1024
#define APP_SERVICE_NAME    "_ca-thingsboard-coap._udp.default.service.arpa"
#define APP_SERVICE_PORT    5683
extern const char APP_MESSAGE_TEMPLATE[];
extern const char APP_URI_TEMPLATE[];
extern const char APP_TOKEN[];

// Thread Nwk config

#define APP_NETWORK_NAME    "Test_nwk"
#define APP_NETWORK_CHANNEL 11
#define APP_NETWORK_PANID   0xAAAA

#define APP_NETWORK_EPANID  {0xAB, 0xCD, 0x00, 0xAB, 0xCD, 0x00, 0xAB, 0xCD};

#define APP_NETWORK_KEY                                                                            \
    {0x00,                                                                                         \
     0x11,                                                                                         \
     0x22,                                                                                         \
     0x33,                                                                                         \
     0x44,                                                                                         \
     0x55,                                                                                         \
     0x66,                                                                                         \
     0x77,                                                                                         \
     0x88,                                                                                         \
     0x99,                                                                                         \
     0xaa,                                                                                         \
     0xbb,                                                                                         \
     0xcc,                                                                                         \
     0xdd,                                                                                         \
     0xee,                                                                                         \
     0xee};
#endif
