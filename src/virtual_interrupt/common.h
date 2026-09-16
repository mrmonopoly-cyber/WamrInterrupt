#pragma once

#include <assert.h>
#include <bits/types/sigset_t.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "wasm_export.h"
#include "logger.h" //INFO: used by other modules

//============================================macros============================================

#ifndef VI_COMMON_PREFIX
#define VI_COMMON_PREFIX static inline
#endif // !VI_COMMON_PREFIX

#define VI_ERROR_WAMR_NO_EXCEPTION  ""

#define VI_DEFAULT_SIG_SUSPEND  SIGPOLL
#define VI_DEFAULT_SIG_RESUME   SIGCONT

#define VI_RESULT_TYPE __attribute__((warn_unused_result))

//============================================types============================================

typedef enum __VirtualInterruptError
{
    VIError_None=0,
    VIError_InvalidInput,
    VIError_Queue,
    VIError_WAMR,
    VIError_Libc,           /* check errno */
}VIError;

typedef enum
{
    VISignals_Suspend,
    VISignals_Resume,

    VISignals_Count
}VISignals;

//============================================declarations======================================

VI_COMMON_PREFIX void _vi_set_errno(int error);
VI_COMMON_PREFIX void _vi_set_wamr_exception(wasm_module_inst_t module_inst);
VI_COMMON_PREFIX bool _vi_exists_wamr_exception(void);
VI_COMMON_PREFIX const char* _vi_get_wamr_exception(void);
VI_COMMON_PREFIX void _vi_clear_wamr_exception(void);
VI_COMMON_PREFIX VIError _vi_set_signal(VISignals signal, int val) VI_RESULT_TYPE;
VI_COMMON_PREFIX VISignals _vi_get_signal(VISignals signal);
VI_COMMON_PREFIX const char* _vi_get_signal_name(VISignals signal);
VI_COMMON_PREFIX VIError _vi_enable_signal(VISignals signal) VI_RESULT_TYPE;
VI_COMMON_PREFIX VIError _vi_disable_all_signals(void) VI_RESULT_TYPE;
VI_COMMON_PREFIX const char* vi_error_to_str(const VIError err);


//=============================================implementation==================================

#define log(...) vi_log(VILoggerLevel_Info, log_buffer, sizeof(log_buffer), "Common", __VA_ARGS__)
#define log_warn(...) vi_log(VILoggerLevel_Warning, log_buffer, sizeof(log_buffer), "Common", __VA_ARGS__)
#define log_err(...) vi_log(VILoggerLevel_Error, log_buffer, sizeof(log_buffer), "Common", __VA_ARGS__)

VI_COMMON_PREFIX void _vi_set_errno(int error)
{
    extern int VI_ERROR_ERRNO;

    VI_ERROR_ERRNO = error;
    errno = error;
}

VI_COMMON_PREFIX void _vi_set_wamr_exception(wasm_module_inst_t module_inst)
{
    extern const char* VI_ERROR_WAMR_EXCEPTION;

    VI_ERROR_WAMR_EXCEPTION = wasm_runtime_get_exception(module_inst);
}

VI_COMMON_PREFIX bool _vi_exists_wamr_exception(void)
{
    extern const char* VI_ERROR_WAMR_EXCEPTION;

    return strcmp(VI_ERROR_WAMR_EXCEPTION,VI_ERROR_WAMR_NO_EXCEPTION);
}

VI_COMMON_PREFIX const char* _vi_get_wamr_exception(void)
{
    extern const char* VI_ERROR_WAMR_EXCEPTION;

    return VI_ERROR_WAMR_EXCEPTION;
}

VI_COMMON_PREFIX void _vi_clear_wamr_exception(void)
{
    extern const char* VI_ERROR_WAMR_EXCEPTION;

    VI_ERROR_WAMR_EXCEPTION = VI_ERROR_WAMR_NO_EXCEPTION;
}

VI_COMMON_PREFIX VIError _vi_set_signal(VISignals signal, int val)
{
    char log_buffer[64] = {0};

    sigset_t set;
    extern int VI_SIGNALS[VISignals_Count];

    if ( signal >= VISignals_Count )
    {
        return VIError_InvalidInput;
    }

    sigemptyset(&set);

    if ( sigaddset(&set, val) == -1 && errno == EINVAL )
    {
        _vi_set_errno(errno);
        return VIError_Libc;
    }

    for (size_t i=0; i<VISignals_Count; i++)
    {
        if ( i != signal && VI_SIGNALS[i] == val )
        {
            return VIError_InvalidInput;
        }
    }

    log("setting signal %s: %d", _vi_get_signal_name(signal), val);
    VI_SIGNALS[signal] = val;

    return VIError_None;
}

VI_COMMON_PREFIX VISignals _vi_get_signal(VISignals signal)
{
    extern int VI_SIGNALS[VISignals_Count];

    return VI_SIGNALS[signal];
}

VI_COMMON_PREFIX const char* _vi_get_signal_name(VISignals signal)
{
    switch (signal)
    {
        case VISignals_Suspend:     return "Suspend";
        case VISignals_Resume:      return "Resume";
        case VISignals_Count:       assert(0 && "unreachable");
    }

    assert(0 && "unreachable");
}

VI_COMMON_PREFIX VIError _vi_enable_signal(VISignals signal)
{
    sigset_t set;

    if ( signal >= VISignals_Count )
    {
        return VIError_InvalidInput;
    }

    sigemptyset(&set);
    sigaddset(&set, _vi_get_signal(signal));

    if ( pthread_sigmask(SIG_UNBLOCK, &set, NULL) < 0 )
    {
        _vi_set_errno(errno);
        return VIError_Libc;
    }

    return VIError_None;
}

VI_COMMON_PREFIX VIError _vi_disable_all_signals(void)
{
    sigset_t set;

    sigfillset(&set);

    if ( pthread_sigmask(SIG_BLOCK, &set, NULL) < 0 )
    {
        _vi_set_errno(errno);
        return VIError_Libc;
    }

    return VIError_None;
}

VI_COMMON_PREFIX const char* vi_error_to_str(const VIError err)
{
    extern int VI_ERROR_ERRNO;
    switch (err)
    {
        case VIError_None:                  return "";
        case VIError_InvalidInput:          return "invalid input";
        case VIError_Queue:                 return "Internal Queue error: Full?";
        case VIError_WAMR:                  return "wamr error";
        case VIError_Libc:                  return "libc error";
    }

    assert(0 && "unreachable");
}

#undef log
#undef log_warn
#undef log_err
