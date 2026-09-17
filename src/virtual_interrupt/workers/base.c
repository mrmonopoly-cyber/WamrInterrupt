#include "base.h"

#include <signal.h>
#include <pthread.h>
#include <unistd.h>

//====================================implementation==============================================

VIError vi_worker_status_init(
        VIWorkerStatus* const restrict status,
        void* (*worker_fun)(void* arg),
        void* arg
        )
{
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

    return VIError_None;
}

void vi_worker_status_suspend(VIWorkerStatus* const restrict status)
{
    assert( status );

    if ( atomic_exchange(&status->working_status, WorkerStatus_Suspended) != WorkerStatus_Suspended )
    {
        pthread_kill(status->th_id, _vi_get_signal(VISignals_Suspend));
        //FIXME: on small intervals it's possible that two requests happen too fast
        //not giving enough time to the kernel to do the context switch for the threads.
        //For now a delay has been added to limit the damage on such cases but it's NOT a solution
        usleep(1000); //HACK: to give time to the kernel to do the context switch
    }
}

void vi_worker_status_resume(VIWorkerStatus* const restrict status)
{
    assert( status );

    atomic_store(&status->working_status, WorkerStatus_Working);
    pthread_kill(status->th_id, _vi_get_signal(VISignals_Resume));
    usleep(1000); //HACK: to give time to the kernel to do the context switch
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

VIError vi_worker_status_signal(VIWorkerStatus* const restrict status)
{
    assert(status);
    int err;

    if ( (err = pthread_kill(status->th_id, _vi_get_signal(VISignals_Resume)) != 0) )
    {
        _vi_set_errno(err);
        return VIError_Libc;
    }

    return VIError_None;
}

void vi_worker_status_destroy(VIWorkerStatus* const restrict status)
{
    assert(status);

    vi_worker_status_resume(status);
    pthread_cancel(status->th_id);
    pthread_join(status->th_id, NULL);
}
