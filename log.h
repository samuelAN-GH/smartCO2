#ifndef THINGSBOARD_LOG_H
#define THINGSBOARD_LOG_H

#include <openthread/platform/logging.h>
#include <openthread/cli.h>

/**
 * @file
 *
 * @brief      Logging helper functions.
 */

#define ERROR(fmt, ...)                                               \
    do {                                                              \
        otCliOutputFormat("[error] " fmt __VA_OPT__(, ) __VA_ARGS__); \
        otCliOutputFormat("\r\n");                                    \
    }                                                                 \
    while (false)

#define INFO(fmt, ...)                                                \
    do {                                                              \
        otCliOutputFormat("[info] " fmt __VA_OPT__(, ) __VA_ARGS__);  \
        otCliOutputFormat("\r\n");                                    \
    } while (false)

#define ERROR_F(f)                                                    \
    otCliOutputFormat("[error] " f " has failed: %s\r\n", otThreadErrorToString(error));

#endif
