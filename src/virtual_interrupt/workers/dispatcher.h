#pragma once

#include "base.h"

//============================================types===============================================
typedef struct
{
    VIWorkerStatus base;
    atomic_size_t n_requests;
    
    atomic_bool run;
}VIDispatcherStatus;

//============================================declarations========================================
VI_RESULT_TYPE(VIError) vi_dispatcher_status_init(VIDispatcherStatus* const restrict status,
        void* (*dispatcher_fun) (void* arg),
        void* arg);

static inline VI_RESULT_TYPE(VIError) vi_dispatcher_status_signal(VIDispatcherStatus* const restrict status);
static inline size_t vi_dispatcher_get_requests(const VIDispatcherStatus* const status);
static inline void vi_dispatcher_consume_request(VIDispatcherStatus* const status);

void vi_dispatcher_status_destroy(VIDispatcherStatus* const restrict status);

//============================================implementation======================================

static inline VIError vi_dispatcher_status_signal(VIDispatcherStatus* const restrict status)
{
    assert(status);
    atomic_fetch_add(&status->n_requests, 1);
    return vi_worker_status_resume(&status->base);
}

static inline size_t vi_dispatcher_get_requests(const VIDispatcherStatus* const status)
{
    assert(status);
    return atomic_load(&status->n_requests);
}

static inline void vi_dispatcher_consume_request(VIDispatcherStatus* const status)
{
    assert(status);
    atomic_fetch_sub(&status->n_requests, 1);
}
