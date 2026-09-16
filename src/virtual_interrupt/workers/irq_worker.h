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

//================================================macros=========================================
#define log(...) vi_log(VILoggerLevel_Info, log_buffer, sizeof(log_buffer), "Worker", __VA_ARGS__)
#define log_warn(...) vi_log(VILoggerLevel_Warning, log_buffer, sizeof(log_buffer), "Worker", __VA_ARGS__)
#define log_err(...) vi_log(VILoggerLevel_Error, log_buffer, sizeof(log_buffer), "Worker", __VA_ARGS__)

//================================================types==========================================
typedef wasm_function_inst_t IrqFuncHandler;

typedef struct
{
    VIWorkerStatus base;
    VIDispatcherStatus *p_dispatcher;

    IrqFuncHandler* p_funcs;
    atomic_size_t func_index;

    wasm_exec_env_t th_exec_env;

    atomic_bool run;
}VIIrqWorkerStatus;

typedef struct
{
    wasm_module_inst_t module_inst;
    VIIrqWorkerStatus* status;
    atomic_int* out;
}ThWorkersArg;

typedef SPAN_TEMPLATE(VIIrqWorkerStatus) VISpanWorkerStatus;

//==============================================declarations======================================
static inline void vi_irq_worker_destroy(VIIrqWorkerStatus* const restrict status);

static inline VIError vi_irq_worker_init(
        VIIrqWorkerStatus* const restrict status,
        VIDispatcherStatus* const restrict p_status_dispatcher,
        IrqFuncHandler* p_funcs,
        wasm_module_inst_t module_inst
        ) VI_RESULT_TYPE;
static inline WorkerStatus vi_irq_worker_get_mode(VIIrqWorkerStatus* const restrict status);
static inline void vi_irq_worker_suspend(VIIrqWorkerStatus* const restrict status);
static inline void vi_irq_worker_resume(VIIrqWorkerStatus* const restrict status);
static inline void vi_irq_worker_destroy(VIIrqWorkerStatus* const restrict status);

//==========================================implementations======================================
static void* _th_irq_worker(void* arg);

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
    atomic_init(&status->run, true);

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

static inline void vi_irq_worker_destroy(VIIrqWorkerStatus* const restrict status)
{
    assert(status);

    atomic_store(&status->run, false);
    vi_irq_worker_resume(status);

    while( vi_irq_worker_get_mode(status) != WorkerStatus_Dead )
    {
        usleep(1000);
    }

    vi_worker_status_destroy(&status->base);
}

static void _th_irq_workder_thread_cleanup(void* arg)
{
    char log_buffer[64] = {0};
    assert(arg);
    VIIrqWorkerStatus *status = arg;

    assert(status);
    wasm_runtime_destroy_exec_env(status->th_exec_env);
    wasm_runtime_destroy_thread_env();

    vi_worker_status_set_working_mode(&status->base, WorkerStatus_Dead);
    log("dead");
}

//===========================================private=============================================

static void* _th_irq_worker(void* arg)
{
    uintptr_t res = VIError_None;
    ThWorkersArg th_arg = *(ThWorkersArg* )arg;

    VIIrqWorkerStatus* status = th_arg.status;
    wasm_module_inst_t module_inst = th_arg.module_inst;
    char log_buffer[128] = {0};
    bool fail = false;

//========================================init=================================================
    wasm_runtime_init_thread_env();
    pthread_cleanup_push(_th_irq_workder_thread_cleanup, th_arg.status);

    if( !module_inst )
    {
        res = VIError_InvalidInput;
        goto end;
    }

    status->th_exec_env = wasm_runtime_create_exec_env(module_inst, 16 << 10); //16 KB
    if ( !status->th_exec_env )
    {
        res = VIError_WAMR;
        fail = true;
        goto end;
    }

    if ( (res = _vi_enable_signal(VISignals_Suspend)) != VIError_None )
    {
        fail = true;
        goto end;
    }

    atomic_store(th_arg.out, res);

//=======================================logic=================================================
    while( 1 )
    {
        assert(th_arg.status->p_dispatcher);

        vi_worker_status_set_working_mode(&th_arg.status->base, WorkerStatus_Done);
        vi_worker_status_self_suspend();

        if ( !atomic_load(&th_arg.status->run) )
        {
            log("received request to terminate execution");
            break;
        }

        size_t func_index = atomic_load(&status->func_index);

        log("calling func: %zu", func_index);
        vi_worker_status_set_working_mode(&th_arg.status->base, WorkerStatus_Working);

        if ( status->p_funcs && status->p_funcs[func_index] )
        {
            if ( !wasm_runtime_call_wasm(status->th_exec_env, status->p_funcs[func_index], 0, NULL) )
            {
                _vi_set_wamr_exception(module_inst);
            }
        }
        else
        {
            log_warn("calling unset irq handler %zu. Skipping", func_index);
        }

        log("finshed func: %zu", func_index);

        if ( (res = vi_dispatcher_status_signal(th_arg.status->p_dispatcher)) )
        {
            log_err("failed signaling the dispatcher: %s", vi_error_to_str(res));
        }
    }

    //INFO: if we reach here it means that there is almost certain a problem
    //or the hole program ended

//=========================================end==================================================
end:
    if ( fail )
    {
        atomic_store(th_arg.out, res);
    }
    pthread_cleanup_pop(true);
    return (void*) res;
}

#undef log
#undef log_warn
#undef log_err
