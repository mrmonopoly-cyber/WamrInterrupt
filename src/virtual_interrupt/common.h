#pragma once

#include "wasm_export.h"
#include <errno.h>
#include <signal.h>

#ifndef SIG_PREEMPTION_WORKERS
#define SIG_PREEMPTION_WORKERS SIGPOLL
#endif

#ifndef SIG_RESUME_WORKERS
#define SIG_RESUME_WORKERS SIGCONT
#endif

#ifndef READY_QUEUE_CAP
#define READY_QUEUE_CAP 32
#endif

#ifndef WAIT_QUEUE_CAP
#define WAIT_QUEUE_CAP 32
#endif

#if SIG_PREEMPTION_WORKERS == SIG_RESUME_WORKERS
#error "SIG_PREEMPTION_WORKERS cannot be the same of SIG_RESUME_WORKERS"
#endif

typedef enum __VirtualInterruptError
{
    VIError_None=0,
    VIError_InvalidInput,
    VIError_Queue,
    VIError_WAMR,
    VIError_Libc,           /* check errno */
}VIError;


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
