#pragma once

#include "base.h"
#include <assert.h>
#include <stdatomic.h>

typedef struct
{
    VIWorkerStatus base;
    atomic_size_t n_requests;
}VIDispatcherStatus;

static inline VIError vi_dispatcher_status_init(VIDispatcherStatus* const restrict status,
        void* (*dispatcher_fun) (void* arg),
        void* arg)
{
    assert(status);

    atomic_init(&status->n_requests, 0);
    return vi_worker_status_init(&status->base, dispatcher_fun , arg);
}

static inline void vi_dispatcher_status_signal(VIDispatcherStatus* const restrict status)
{
    assert(status);
    vi_worker_status_signal(&status->base);
}

static inline void vi_dispatcher_status_destroy(VIDispatcherStatus* const restrict status)
{
    assert(status);
    return vi_worker_status_destroy(&status->base);
}
