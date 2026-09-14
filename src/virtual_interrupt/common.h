#pragma once

#include <assert.h>
#include <bits/types/sigset_t.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "wasm_export.h"

#define VI_ERROR_WAMR_NO_EXCEPTION  ""

#ifndef READY_QUEUE_CAP
#define READY_QUEUE_CAP 32
#endif

#ifndef WAIT_QUEUE_CAP
#define WAIT_QUEUE_CAP 32
#endif

#define VI_DEFAULT_SIG_SUSPEND  SIGPOLL
#define VI_DEFAULT_SIG_RESUME   SIGCONT

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

    __VISignals_Count
}VISignals;

static inline void _vi_set_errno(int error)
{
    extern int VI_ERROR_ERRNO;

    VI_ERROR_ERRNO = error;
    errno = error;
}

static inline void _vi_set_wamr_exception(wasm_module_inst_t module_inst)
{
    extern const char* VI_ERROR_WAMR_EXCEPTION;

    VI_ERROR_WAMR_EXCEPTION = wasm_runtime_get_exception(module_inst);
}

static inline bool _vi_exists_wamr_exception()
{
    extern const char* VI_ERROR_WAMR_EXCEPTION;

    return strcmp(VI_ERROR_WAMR_EXCEPTION,VI_ERROR_WAMR_NO_EXCEPTION);
}

static inline const char* _vi_get_wamr_exception()
{
    extern const char* VI_ERROR_WAMR_EXCEPTION;

    return VI_ERROR_WAMR_EXCEPTION;
}

static inline void _vi_clean_wamr_exception()
{
    extern const char* VI_ERROR_WAMR_EXCEPTION;

    VI_ERROR_WAMR_EXCEPTION = VI_ERROR_WAMR_NO_EXCEPTION;
}

static inline VIError _vi_set_signal(VISignals signal, int val)
{
    sigset_t set;
    extern int VI_SIGNALS[__VISignals_Count];

    if ( signal >= __VISignals_Count )
    {
        return VIError_InvalidInput;
    }

    sigemptyset(&set);

    if ( sigaddset(&set, val) == -1 && errno == EINVAL )
    {
        _vi_set_errno(errno);
        return VIError_Libc;
    }

    for (size_t i=0; i<__VISignals_Count; i++)
    {
        if ( i != signal && VI_SIGNALS[i] == val )
        {
            return VIError_InvalidInput;
        }
    }

    VI_SIGNALS[signal] = val;

    return VIError_None;
}

static inline VISignals _vi_get_signal(VISignals signal)
{
    extern int VI_SIGNALS[__VISignals_Count];

    assert(signal < __VISignals_Count);

    return VI_SIGNALS[signal];
}
