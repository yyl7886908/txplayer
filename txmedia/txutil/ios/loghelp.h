#ifndef TXUTIL_IOS__LOGHELP_H
#define TXUTIL_IOS__LOGHELP_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TX_LOG_TAG "TXMEDIA"

#define TX_LOG_UNKNOWN     0
#define TX_LOG_DEFAULT     1

#define TX_LOG_VERBOSE     2
#define TX_LOG_DEBUG       3
#define TX_LOG_INFO        4
#define TX_LOG_WARN        5
#define TX_LOG_ERROR       6
#define TX_LOG_FATAL       7
#define TX_LOG_SILENT      8

#define VLOG(level, TAG, ...)    ((void)vprintf(__VA_ARGS__))
#define VLOGV(...)  VLOG(TX_LOG_VERBOSE,   TX_LOG_TAG, __VA_ARGS__)
#define VLOGD(...)  VLOG(TX_LOG_DEBUG,     TX_LOG_TAG, __VA_ARGS__)
#define VLOGI(...)  VLOG(TX_LOG_INFO,      TX_LOG_TAG, __VA_ARGS__)
#define VLOGW(...)  VLOG(TX_LOG_WARN,      TX_LOG_TAG, __VA_ARGS__)
#define VLOGE(...)  VLOG(TX_LOG_ERROR,     TX_LOG_TAG, __VA_ARGS__)

#define ALOG(level, TAG, ...)    ((void)printf(__VA_ARGS__))
#define ALOGV(...)  ALOG(TX_LOG_VERBOSE,   TX_LOG_TAG, __VA_ARGS__)
#define ALOGD(...)  ALOG(TX_LOG_DEBUG,     TX_LOG_TAG, __VA_ARGS__)
#define ALOGI(...)  ALOG(TX_LOG_INFO,      TX_LOG_TAG, __VA_ARGS__)
#define ALOGW(...)  ALOG(TX_LOG_WARN,      TX_LOG_TAG, __VA_ARGS__)
#define ALOGE(...)  ALOG(TX_LOG_ERROR,     TX_LOG_TAG, __VA_ARGS__)
#define LOG_ALWAYS_FATAL(...)   do { ALOGE(__VA_ARGS__); exit(1); } while (0)

#ifdef __cplusplus
}
#endif

#endif
