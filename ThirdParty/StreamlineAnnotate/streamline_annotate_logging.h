/* Copyright (C) 2021 by Arm Limited. All rights reserved. */

#ifndef STREAMLINE_ANNOTATE_LOGGING_H
#define STREAMLINE_ANNOTATE_LOGGING_H

//Mapped to android log levels - android_LogPriority
enum log_levels {
    LOG_UNKNOWN = 0,
    LOG_DEFAULT,
    LOG_VERBOSE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
    LOG_SILENT
};

/* ANDROID IMPLEMENTATION */
#if defined(ANDROID) || defined(__ANDROID__)
#include <android/log.h>

#define LOG_TAG "AnnotationLog"

#define LOGGING(LOG_LEVEL, fmt, ...)                                                                                   \
    __android_log_print(LOG_LEVEL, LOG_TAG, "%s/%s:%d " fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

/* LINUX IMPLEMENTATION */
#elif defined(linux) || defined(__linux) || defined(__linux__)
// clang-format off
char *log_levels[] = { "UNKNOWN",
                       "DEFAULT",
                       "VERBOSE",
                       "DEBUG",
                       "INFO",
                       "WARN",
                       "ERROR",
                       "FATAL",
                       "SILENT"};
// clang-format on
#define LOGGING(LOG_LEVEL, fmt, ...)                                                                                   \
    printf("%s/%s:%d [%s] " fmt " \n", __FILE__, __func__, __LINE__, log_levels[LOG_LEVEL], ##__VA_ARGS__);

#endif
//Use to do logging, if not needed un-define this variable
#define ENABLE_LOG

#ifdef ENABLE_LOG
#define LOG(LOG_LEVEL, fmt, ...) LOGGING(LOG_LEVEL, fmt, ##__VA_ARGS__)
#else
#define LOG(LOG_LEVEL, fmt, ...) // nothing
#endif

#endif /* STREAMLINE_ANNOTATE_LOGGING_H */
