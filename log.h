#ifndef THINGSBOARD_LOG_H
#define THINGSBOARD_LOG_H

#include <openthread/platform/logging.h>
#include <openthread/cli.h>
#include <SEGGER_RTT.h>
#include "ustimer.h"

/**
 * @file
 *
 * @brief      Logging helper functions.
 */

#define ERROR(fmt, ...)                                                                         \
    do {                                                                                        \
        SEGGER_RTT_printf(0, RTT_CTRL_TEXT_BRIGHT_RED"[ERROR]" fmt __VA_OPT__(, ) __VA_ARGS__); \
        USTIMER_Delay(1000);                                                                    \
        SEGGER_RTT_printf(0, "\r\n");                                                           \
    }                                                                                           \
    while (false)

#define INFO(fmt, ...)                                                  \
    do {                                                                \
        SEGGER_RTT_printf(0, "[INFO]" fmt __VA_OPT__(, ) __VA_ARGS__);  \
        USTIMER_Delay(1000);                                            \
        SEGGER_RTT_printf(0, "\r\n");                                   \
    }                                                                   \
    while (false)

#define ERROR_F(f)                                                    \
    otCliOutputFormat("[error] " f " has failed: %s\r\n", otThreadErrorToString(error));

#endif
