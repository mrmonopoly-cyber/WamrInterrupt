#pragma once

#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>

#include "base.h"
#include "dispatcher.h"
#include "wasm_export.h"

#include "../common.h"

#include "../span/span.h"

typedef wasm_function_inst_t IrqFuncHandler;

typedef struct
{
    VIWorkerStatus base;
    VIDispatcherStatus *p_dispatcher;

    IrqFuncHandler* p_funcs;
    atomic_size_t func_index;
}VIIrqWorkerStatus;

typedef struct
{
    wasm_module_inst_t module_inst;
    VIIrqWorkerStatus* status;
    atomic_int* out;
}ThWorkersArg;

typedef SPAN_TEMPLATE(VIIrqWorkerStatus) VISpanWorkerStatus;

static void* _th_irq_worker(void* arg);
static inline void vi_irq_worker_destroy(VIIrqWorkerStatus* const restrict status);

static inline VIError vi_irq_worker_init(
        VIIrqWorkerStatus* const restrict status,
        VIDispatcherStatus* const restrict p_status_dispatcher,
        IrqFuncHandler* p_funcs,
        wasm_module_inst_t module_inst
        )
{
    assert(status && p_funcs);

    atomic_int out;

    ThWorkersArg arg = 
    {
        .module_inst = module_inst,
        .status = status,
        .out = &out,
    };

    atomic_init(&out, -1);

    atomic_init(&status->func_index, 0);

    status->p_funcs = p_funcs;
    status->p_dispatcher = p_status_dispatcher;

    VIError res = vi_worker_status_init(&status->base, _th_irq_worker, &arg);

    if ( res != VIError_None )
    {
        return res;
    }

    while ( atomic_load(&out) == -1 )
    {
        usleep(1000);
    }

    if ( out != VIError_None )
    {
        vi_irq_worker_destroy(status);
    }


    return res;
}

static inline WorkerStatus vi_irq_worker_get_mode(VIIrqWorkerStatus* const restrict status)
{
    assert(status);
    return vi_worker_status_get_working_mode(&status->base);
}

static inline void vi_irq_worker_destroy(VIIrqWorkerStatus* const restrict status)
{
    assert(status);
    vi_worker_status_destroy(&status->base);
}

static inline void vi_irq_worker_suspend(VIIrqWorkerStatus* const restrict status)
{
    assert(status);
    vi_worker_status_suspend(&status->base);
}

static inline void vi_irq_worker_resume(VIIrqWorkerStatus* const restrict status)
{
    assert(status);

    vi_worker_status_resume(&status->base);
}

//==========================================implementations======================================

static void _th_irq_workder_thread_cleanup(void* arg)
{
    assert(arg);
    wasm_exec_env_t *th_exec_env = arg;

    assert(*th_exec_env);
    wasm_runtime_destroy_exec_env(*th_exec_env);
    wasm_runtime_destroy_thread_env();
}

static void* _th_irq_worker(void* arg)
{
    uintptr_t res = VIError_None;
    ThWorkersArg th_arg = *(ThWorkersArg* )arg;

    VIIrqWorkerStatus* status = th_arg.status;
    wasm_module_inst_t module_inst = th_arg.module_inst;
    wasm_exec_env_t th_exec_env = {0};
    sigset_t set = {0};
    int err;
    char log_buffer[128] = {0};

//========================================init=================================================
    pthread_cleanup_push(_th_irq_workder_thread_cleanup, &th_exec_env);
    wasm_runtime_init_thread_env();

    if( !module_inst )
    {
        res = VIError_InvalidInput;
        goto end;
    }

    th_exec_env = wasm_runtime_create_exec_env(module_inst, 16 << 10); //16 KB
    if ( !th_exec_env )
    {
        res = VIError_WAMR;
        goto end;
    }

    sigemptyset(&set);
    sigaddset(&set, _vi_get_signal(VISignals_Suspend));
    if ( ( err =pthread_sigmask(SIG_UNBLOCK, &set, NULL) ) < 0 )
    {
        res = VIError_Libc;
        _vi_set_errno(err);
        goto end;
    }

    atomic_store(th_arg.out, VIError_None);

//=======================================logic=================================================
    while(1)
    {
        assert(th_arg.status->p_dispatcher);

        vi_worker_status_set_working_mode(&th_arg.status->base, WorkerStatus_Done);
        vi_worker_status_self_suspend();

        size_t func_index = atomic_load(&status->func_index);

        assert(status->p_funcs);

        vi_log(VILoggerLevel_Info, log_buffer, sizeof(log_buffer),
                "VIWorker: calling func: %zu", func_index);
        vi_worker_status_set_working_mode(&th_arg.status->base, WorkerStatus_Working);
        if ( !wasm_runtime_call_wasm(th_exec_env, status->p_funcs[func_index], 0, NULL) )
        {
            _vi_set_wamr_exception(module_inst);
        }
        vi_log(VILoggerLevel_Info, log_buffer, sizeof(log_buffer),
                "VIWorker: finshed func: %zu", func_index);

        vi_dispatcher_status_signal(th_arg.status->p_dispatcher);
    }

    //INFO: if we reach here it means that there is almost certain a problem
    //or the hole program ended

//=========================================end==================================================
end:
    atomic_store(th_arg.out, res);
    pthread_cleanup_pop(true);
    return (void*) res;
}

#undef printf
