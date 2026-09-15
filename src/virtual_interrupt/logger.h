#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>

#ifndef VILOGGER_PREFIX
#define VILOGGER_PREFIX
#endif // !VILOGGER_PREFIX

#define VILOGGER_PRINTF_FORMAT(STRING_INDEX, FIRST_TO_CHECK) \
    __attribute__ ((format (printf, STRING_INDEX, FIRST_TO_CHECK)))

#define VILOGGER_FILE_NAME "vi_log"

#define printf(FMT, ...) static_assert(0, "printf is not allowed use vi_log");

typedef enum
{
    VILoggerLevel_Trace,
    VILoggerLevel_Debug,
    VILoggerLevel_Info,
    VILoggerLevel_Warning,
    VILoggerLevel_Error,
}VILoggerLevel;

extern VILoggerLevel vi_minimal_log_level;

typedef void (VILogHandler) (VILoggerLevel level, const char* msg);

VILOGGER_PREFIX void vi_log(
        VILoggerLevel level,
        char* buffer,
        size_t buffer_len,
        const char *fmt,
        ...)VILOGGER_PRINTF_FORMAT(4, 5);

VILOGGER_PREFIX void vi_set_log_handler(VILogHandler* handler);
VILOGGER_PREFIX VILogHandler* vi_get_log_handler(void);

void vi_default_log_handler(VILoggerLevel level, const char* msg);
