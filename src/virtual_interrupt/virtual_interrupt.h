#pragma once

#include <assert.h>
#include <pthread.h>
#include <stddef.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>

#include "wasm_export.h"
#include "spscq/spscq.h"

#define SIG_PREEMPTION_WORKERS SIGPOLL

typedef enum __VirtualInterruptError
{
    VIError_None=0,
    VIError_InvalidInput,
    VIError_Queue,
    VIError_WAMR,
    VIError_Libc,           /* check errno */
}VIError;

typedef int32_t IrqLine;
typedef void (*IrqFuncHandler) (wasm_exec_env_t exec_env);
typedef TEMPLATE_SPSCQ(IrqLine, 32) SPSCQ_UReq;

typedef struct
{
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    pthread_t tid;
}VIPreemptionStatus;

typedef struct
{
    pthread_cond_t* p_dispatcher_cond;
    pthread_mutex_t* p_dispatcher_mutex;
}VIDispatcherSignalRef;

typedef struct __VirtualInterruptDispatcher
{
    struct VIMainFunStatus
    {
        VIPreemptionStatus preemption_status;
        VIDispatcherSignalRef dispatcher_signal_ref;
        bool working;

    }main_f_status;

    struct VIWorkerStatus
    {
        pthread_cond_t data_cond;
        pthread_mutex_t data_mutex;

        VIPreemptionStatus preemption_status;

        size_t func_index;
        atomic_bool working;

        VIDispatcherSignalRef dispatcher_signal_ref;

        IrqFuncHandler* p_funcs;
    }*workers;
    size_t n_workers;

    IrqFuncHandler* funcs;
    size_t n_funcs;

    pthread_cond_t dispatcher_cond;
    pthread_mutex_t dispatcher_mutex;
    SPSCQ_UReq channel_ureq;
    pthread_t dispatcher_tid;
    size_t executing_worker; //INFO: 0 means None, K means workers[k-1] IS CURRENTLY EXECUTING

}VIDispatcher;


VIError vidispatcher_init(
        VIDispatcher* const restrict dispatcher,
        wasm_module_inst_t module_inst,
        wasm_function_inst_t main_f,
        const size_t n_lines,
        const size_t depth);

VIError vidispatcher_assign_irq_to_line(
        VIDispatcher* const restrict dispatcher,
        const IrqFuncHandler irq_handler,
        const size_t line);

VIError vidispatcher_start(VIDispatcher* const restrict dispatcher);

VIError vidispatcher_trigger_interrupt(VIDispatcher* const restrict dispatcher, IrqLine line);

void vidispatcher_destroy(VIDispatcher* const restrict dispatcher);

static inline const char* vi_error_to_str(const VIError err)
{
    extern int VI_ERROR_ERRNO;
    switch (err)
    {
        case VIError_None:                  return "";
        case VIError_InvalidInput:          return "invalid input";
        case VIError_Queue:                 return "Internal Queue error: Full?";
        case VIError_WAMR:                  return "wamr error";
        case VIError_Libc:                  return strerror(VI_ERROR_ERRNO);
    }

    assert(0 && "unreachable");
}
