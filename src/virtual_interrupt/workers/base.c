#include "base.h"

#include <signal.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

#define log(...) vi_log(VILoggerLevel_Info, log_buffer, sizeof(log_buffer), "Worker", __VA_ARGS__)
#define log_warn(...) vi_log(VILoggerLevel_Warning, log_buffer, sizeof(log_buffer), "Worker", __VA_ARGS__)
#define log_err(...) vi_log(VILoggerLevel_Error, log_buffer, sizeof(log_buffer), "Worker", __VA_ARGS__)

//====================================implementation==============================================

VIError vi_worker_status_init(
        VIWorkerStatus* const restrict status,
        void* (*worker_fun)(void* arg),
        void* arg
        )
{
    char log_buffer[32] = {0};
    assert(status && worker_fun);

    atomic_init(&status->working_status, WorkerStatus_Init);

    int err =pthread_create(
            &status->th_id,
            NULL, 
            worker_fun,
            arg);

    if ( err != 0 )
    {
        _vi_set_errno(err);
        return VIError_Libc;
    }

    log("starting new Worker: %zu", status->th_id);

    return VIError_None;
}

VIError vi_worker_status_suspend(VIWorkerStatus* const restrict status)
{
    char log_buffer[32] = {0};
    assert( status );

    if ( atomic_exchange(&status->working_status, WorkerStatus_Suspended) != WorkerStatus_Suspended )
    {
        log("suspending Worker: %zu", status->th_id);

        pthread_kill(status->th_id, _vi_get_signal(VISignals_Suspend));
        return VIError_None;
    }

    return VIError_Async;
}

VIError vi_worker_status_resume(VIWorkerStatus* const restrict status)
{
    char log_buffer[32] = {0};
    assert( status );
    
    if ( atomic_exchange(&status->working_status, WorkerStatus_Working) != WorkerStatus_Working )
    {
        log("resuming Worker: %zu", status->th_id);
        atomic_store(&status->working_status, WorkerStatus_Working);
        pthread_kill(status->th_id, _vi_get_signal(VISignals_Resume));
        return VIError_None;
    }

    return VIError_Async;
}

void vi_worker_status_set_working_mode(
        VIWorkerStatus* const restrict status,
        WorkerStatus wc)
{
    assert( status && wc < __WorkerStatus_Count );
    atomic_store(&status->working_status, wc);
}

WorkerStatus vi_worker_status_get_working_mode( const VIWorkerStatus* const restrict status)
{
    assert( status );
    size_t res = atomic_load(&status->working_status);

    return (WorkerStatus) res;
}

VIError vi_worker_status_self_suspend(void)
{
    sigset_t set = {0};
    sigfillset(&set);
    sigdelset(&set, _vi_get_signal(VISignals_Resume));
    sigsuspend(&set); //NOLINT(concurrency-mt-unsafe)

    return VIError_None;
}

void vi_worker_status_destroy(VIWorkerStatus* const restrict status)
{
    assert(status);

    (void) vi_worker_status_resume(status);
    pthread_cancel(status->th_id);
    pthread_join(status->th_id, NULL);
}
