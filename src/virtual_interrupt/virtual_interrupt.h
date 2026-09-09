#pragma once

#include <assert.h>
#include <pthread.h>
#include <stddef.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>

#include "minheap/minheap.h"
#include "wasm_export.h"
#include "spscq/spscq.h"

#ifndef SIG_PREEMPTION_WORKERS
#define SIG_PREEMPTION_WORKERS SIGPOLL
#endif

#ifndef READY_QUEUE_CAP
#define READY_QUEUE_CAP 32
#endif

#ifndef WAIT_QUEUE_CAP
#define WAIT_QUEUE_CAP 32
#endif

typedef enum __VirtualInterruptError
{
    VIError_None=0,
    VIError_InvalidInput,
    VIError_Queue,
    VIError_WAMR,
    VIError_Libc,           /* check errno */
}VIError;

typedef size_t IrqLine;
typedef void (*IrqFuncHandler) (wasm_exec_env_t exec_env);
typedef TEMPLATE_SPSCQ(IrqLine, READY_QUEUE_CAP) SPSCQ_UReq;
typedef MINHEAP_TEMPLATE(IrqLine, WAIT_QUEUE_CAP) MinheapUReq;

typedef struct
{
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    pthread_t tid;
}VIPreemptionStatus;

typedef struct
{
    pthread_cond_t cond;
    pthread_mutex_t mutex;
}VIDispatcherSignalStatus;

typedef struct __VirtualInterruptDispatcher
{
    struct VIMainFunStatus
    {
        VIPreemptionStatus preemption_status;
        VIDispatcherSignalStatus* p_dispatcher_signal_status;
        bool working;

    }main_f_status;

    struct VIWorkerStatus
    {
        pthread_cond_t data_cond;
        pthread_mutex_t data_mutex;

        VIPreemptionStatus preemption_status;

        size_t func_index;
        atomic_bool working;

        VIDispatcherSignalStatus* p_dispatcher_signal_status;

        IrqFuncHandler* p_funcs;
    }*workers;
    size_t n_workers;

    IrqFuncHandler* funcs;
    size_t n_funcs;

    VIDispatcherSignalStatus signal_status;

    SPSCQ_UReq channel_ready_ureq;
    MinheapUReq minheap_ureq;

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

VIError vidispatcher_trigger_interrupt(VIDispatcher* const restrict dispatcher, const IrqLine line);

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
