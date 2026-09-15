#pragma once

#include <assert.h>
#include <bits/types/sigset_t.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stddef.h>

#include "../common.h"

typedef enum
{
    WorkerStatus_Init = 0,
    WorkerStatus_Working,
    WorkerStatus_Suspended,
    WorkerStatus_Done,
    WorkerStatus_Dead,

    __WorkerStatus_Count
}WorkerStatus;

typedef size_t ThreadID;

typedef struct
{
    ThreadID th_id;
    atomic_size_t working_status;
}VIWorkerStatus;

static inline VIError vi_worker_status_init(
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

static inline void vi_worker_status_suspend(VIWorkerStatus* const restrict status)
{
    assert( status );

    if ( atomic_exchange(&status->working_status, WorkerStatus_Suspended) != WorkerStatus_Suspended )
    {
        pthread_kill(status->th_id, _vi_get_signal(VISignals_Suspend));
    }
}

static inline void vi_worker_status_resume(VIWorkerStatus* const restrict status)
{
    assert( status );

    atomic_store(&status->working_status, WorkerStatus_Working);
    pthread_kill(status->th_id, _vi_get_signal(VISignals_Resume));
}

static inline void vi_worker_status_set_working_mode(
        VIWorkerStatus* const restrict status,
        WorkerStatus wc)
{
    assert( status && wc < __WorkerStatus_Count );
    atomic_store(&status->working_status, wc);
}

static inline WorkerStatus vi_worker_status_get_working_mode( VIWorkerStatus* const restrict status)
{
    assert( status );
    size_t res = atomic_load(&status->working_status);

    return (WorkerStatus) res;
}

static inline VIError vi_worker_status_self_suspend(void)
{
    sigset_t set = {0};
    sigfillset(&set);
    sigdelset(&set, _vi_get_signal(VISignals_Resume));
    sigsuspend(&set); //NOLINT(concurrency-mt-unsafe)

    return VIError_None;
}

static inline VIError vi_worker_status_signal(VIWorkerStatus* const restrict status)
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

static inline void vi_worker_status_destroy(VIWorkerStatus* const restrict status)
{
    assert(status);

    vi_worker_status_resume(status);
    pthread_cancel(status->th_id);
    pthread_join(status->th_id, NULL);
}
