#include "logger.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>

static FILE* vi__log_file = NULL;
static char vi__log_file_name[256];
static VILogHandler* vi__log_handler = &vi_default_log_handler;
VILoggerLevel vi_minimal_log_level = VILoggerLevel_Info;

VILOGGER_PREFIX void vi_set_log_handler(VILogHandler* handler)
{
    vi__log_handler = handler;
}

VILOGGER_PREFIX VILogHandler* vi_get_log_handler(void)
{
    return vi__log_handler;
}

VILOGGER_PREFIX const char* vi_get_log_file_name(void)
{
    return vi__log_file_name;
}

VILOGGER_PREFIX void vi_log(
        VILoggerLevel level,
        char* buffer,
        size_t buffer_len,
        const char *fmt,
        ...)
{
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, buffer_len, fmt, args);
    va_end(args);
    vi__log_handler(level, buffer);
}

void vi_log_file_init(const char* base_path)
{
    if ( !vi__log_file )
    {
        struct timespec ts = {0};
        struct tm tm_info = {0};

        clock_gettime(CLOCK_REALTIME, &ts);
        localtime_r(&ts.tv_sec, &tm_info);

        snprintf(
                vi__log_file_name,
                sizeof(vi__log_file_name),
                "%s_%zu%zu.log",
                base_path, ts.tv_sec, ts.tv_nsec
                );

        vi__log_file = fopen(vi__log_file_name, "wa");
        assert(vi__log_file);
    }
}

void vi_default_log_handler(VILoggerLevel level, const char* msg)
{
    if ( level < vi_minimal_log_level ) return;

    const char* prefix = NULL;
    char str_time[52] = {0};
    struct timespec ts = {0};
    struct tm tm_info = {0};
    size_t milliseconds = 0;

    clock_gettime(CLOCK_REALTIME, &ts);

    localtime_r(&ts.tv_sec, &tm_info);
    strftime(str_time, sizeof(str_time), "%H:%M:%S", &tm_info);
    milliseconds = ts.tv_nsec / 1000 * 1000;

    switch (level)
    {
        case VILoggerLevel_Trace:
            {
                prefix = "ℹ️ \x1b[40m[TRACE]\x1b[0m";
            }
            break;
        case VILoggerLevel_Debug:
            {
                prefix = "ℹ️ \x1b[38m[DEBUG]\x1b[0m";
            }
            break;
        case VILoggerLevel_Info:
            {
                prefix = "ℹ️ \x1b[36m[INFO]\x1b[0m";
            }
            break;
        case VILoggerLevel_Warning:
            {
                prefix = "⚠️ \x1b[33m[WARNING]\x1b[0m";
            }
            break;
        case VILoggerLevel_Error:
            {
                prefix = "🚨 \x1b[31m[ERROR]\x1b[0m";
            }
            break;
    }

    assert(vi__log_file);

    fprintf(vi__log_file, "%s %s.%03zu: %s\n", prefix, str_time, milliseconds, msg);
}
