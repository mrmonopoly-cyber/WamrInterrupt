#include "main_logic.h"

#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define log(...) vi_log(VILoggerLevel_Info, log_buffer, sizeof(log_buffer), "MainLogic", __VA_ARGS__)
#define log_warn(...) vi_log(VILoggerLevel_Warning, log_buffer, sizeof(log_buffer), "MainLogic", __VA_ARGS__)
#define log_err(...) vi_log(VILoggerLevel_Error, log_buffer, sizeof(log_buffer), "MainLogic", __VA_ARGS__)

static void* _th_main_thread(void* arg);

VIError vi_main_logic_init(
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

void vi_main_logic_suspend(VIMainLogicStatus* const restrict status)
{
    char log_buffer[64] = {0};

    assert(status);

    log("suspending main thread");
    vi_worker_status_suspend(&status->base);
    vi_worker_status_set_working_mode(&status->base, WorkerStatus_Suspended);
}

void vi_main_logic_resume(VIMainLogicStatus* const restrict status)
{
    char log_buffer[64] = {0};

    assert(status);

    log("resuming main thread");
    vi_worker_status_resume(&status->base);
    vi_worker_status_set_working_mode(&status->base, WorkerStatus_Working);
}

WorkerStatus vi_main_logic_get_mode(const VIMainLogicStatus* const restrict status)
{
    assert(status);

    return vi_worker_status_get_working_mode(&status->base);
}

void vi_main_logic_destroy(VIMainLogicStatus* const restrict status)
{
    assert(status);

    vi_worker_status_destroy(&status->base);
}

//==========================================private=============================================

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
    char log_buffer[128] = {0};
    bool fail = false;

    //====================================init=================================================
    pthread_cleanup_push(_th_main_thread_cleanup, &th_exec_env);
    wasm_runtime_init_thread_env();

    if( !module_inst )
    {
        res = VIError_InvalidInput;
        fail = true;
        goto end;
    }

    th_exec_env = wasm_runtime_create_exec_env(module_inst, 16 << 10); //16 KB
    if ( !th_exec_env )
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

    log("suspending");
    vi_worker_status_set_working_mode(&th_arg.status->base, WorkerStatus_Suspended);
    vi_worker_status_self_suspend();

//=======================================logic=================================================
    log("starting main");
    vi_worker_status_set_working_mode(&th_arg.status->base, WorkerStatus_Working);
    if ( wasm_runtime_call_wasm(th_exec_env, th_arg.main_f, 0, NULL) )
    {
        _vi_set_wamr_exception(module_inst);
        log_err("error calling main function: %s", _vi_get_wamr_exception());
    }

    //INFO: if we reach here it means that the main has ended for any reason which is probably
    //an error unless the hole program ended
    log("main ended");
    if( (res=vi_worker_status_signal(&th_arg.status->p_dispatcher_status->base)) != VIError_None )
    {
        char buf[64] = {0};
        strerror_r(errno, buf, sizeof(buf));
        log_err("failed segnaling dispatcher: %s %s", vi_error_to_str(res), buf);
    }
    vi_worker_status_set_working_mode(&th_arg.status->base, WorkerStatus_Done);

//=======================================end==================================================
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
