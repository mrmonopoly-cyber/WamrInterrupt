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

VI_RESULT_TYPE(VIError) vi_dispatcher_status_signal(VIDispatcherStatus* const restrict status);
void vi_dispatcher_status_destroy(VIDispatcherStatus* const restrict status);

