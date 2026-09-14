#pragma once

#include <assert.h>
#include <signal.h>
#include <stdatomic.h>
#include <unistd.h>

#include "base.h"
#include "dispatcher.h"
#include "wasm_export.h"

typedef struct
{
    VIWorkerStatus base;
    VIDispatcherStatus* p_dispatcher_status;
}VIMainLogicStatus;

typedef struct
{
    wasm_module_inst_t module_inst;
    VIMainLogicStatus* status;
    wasm_function_inst_t main_f;
    atomic_int* out;
}ThMainThreadArg;

static void* _th_main_thread(void* arg);

static inline VIError vi_main_logic_init(
        VIMainLogicStatus* const restrict status,
        VIDispatcherStatus* const restrict p_dispatcher_status,
        wasm_function_inst_t f_main,
        wasm_module_inst_t module_inst
        )
{
    VIError res;
    atomic_int out;
    ThMainThreadArg th_arg =
    {
        .main_f = f_main,
        .module_inst = module_inst,
        .out = &out,
        .status = status,
    };
    assert(status && p_dispatcher_status);
    atomic_init(&out, -1);

    status->p_dispatcher_status = p_dispatcher_status;

    if ( (res = vi_worker_status_init(&status->base, _th_main_thread, &th_arg)) != VIError_None )
    {
        return res;
    }

    while( atomic_load(&out) == -1 )
    {
        usleep(100);
    }

    if ( out != VIError_None ) 
    {
        goto fail;
    }

    return out;

fail:
    vi_worker_status_destroy(&status->base);
    return out;
}

static inline void vi_main_logic_suspend(VIMainLogicStatus* const restrict status)
{
    assert(status);
    vi_worker_status_suspend(&status->base);
}

static inline void vi_main_logic_resume(VIMainLogicStatus* const restrict status)
{
    assert(status);
    vi_worker_status_resume(&status->base);
}

static inline WorkerStatus vi_main_logic_get_mode(VIMainLogicStatus* const restrict status)
{
    assert(status);

    return vi_worker_status_get_working_mode(&status->base);
}

static inline void vi_main_logic_destroy(VIMainLogicStatus* const restrict status)
{
    assert(status);

    vi_worker_status_destroy(&status->base);
}



//==============================================implementation================================

static void _th_main_thread_cleanup(void* arg)
{
    assert(arg);
    wasm_exec_env_t *th_exec_env = arg;

    assert(*th_exec_env);
    wasm_runtime_destroy_exec_env(*th_exec_env);
    wasm_runtime_destroy_thread_env();
}

static void* _th_main_thread(void* arg)
{
    uintptr_t res = VIError_None;
    ThMainThreadArg th_arg = *(ThMainThreadArg*) arg;

    wasm_module_inst_t module_inst = th_arg.module_inst;
    wasm_exec_env_t th_exec_env = {0};
    sigset_t set = {0};
    int err;

    //====================================init=================================================
    pthread_cleanup_push(_th_main_thread_cleanup, &th_exec_env);
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

    vi_worker_status_self_suspend();

//=======================================logic=================================================
    th_arg.status->base.working_status = WorkerStatus_Working;
    if ( wasm_runtime_call_wasm(th_exec_env, th_arg.main_f, 0, NULL) )
    {
        _vi_set_wamr_exception(module_inst);
    }

    //INFO: if we reach here it means that the main has ended for any reason which is probably
    //an error unless the hole program ended
    vi_worker_status_signal(&th_arg.status->p_dispatcher_status->base);
    th_arg.status->base.working_status = WorkerStatus_Done;

//=======================================end==================================================
end:
    atomic_store(th_arg.out, res);
    pthread_cleanup_pop(true);
    return (void*) res;
}
