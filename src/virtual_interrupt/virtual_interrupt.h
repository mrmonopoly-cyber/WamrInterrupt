#pragma once

#include <pthread.h>
#include <stddef.h>
#include <signal.h>
#include <stdint.h>

#include "wasm_export.h"
#include "spscq/spscq.h"

#define SIG_PREEMPTION_WORKERS SIGPOLL

typedef enum __VirtualInterruptError
{
    VIError_None=0,
    VIError_InvalidInput,
    VIError_WAMR,
    VIError_Libc,         /* check errno */
}VIError;

typedef void (*IrqFuncHandler) (void);
typedef TEMPLATE_SPSCQ(int32_t, 32) SPSCQ_UReq;

typedef struct __VirtualInterruptDispatcher
{
    struct VIWorkerStatus
    {
        pthread_t tid;

        pthread_cond_t data_cond;
        pthread_mutex_t data_mutex;

        pthread_cond_t preemption_cond;
        pthread_mutex_t preemption_mutex;

        IrqFuncHandler func;
        atomic_bool working;

        pthread_cond_t* p_dispatcher_cond;
        pthread_mutex_t* p_dispatcher_mutex;
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
        const size_t n_lines,
        const size_t depth);

VIError vidispatcher_assign_irq_to_line(
        VIDispatcher* const restrict dispatcher,
        const IrqFuncHandler irq_handler,
        const size_t line);

VIError vidispatcher_start(VIDispatcher* const restrict dispatcher);

void vidispatcher_destroy(VIDispatcher* const restrict dispatcher);
