#pragma once

#include <assert.h>
#include <stdatomic.h>
#include "../common.h"

//==========================================macros================================================

//==========================================types=================================================
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

typedef struct VI_ASYNC_STATE
{
    ThreadID th_id;
    atomic_size_t working_status;
}VIWorkerStatus;

//====================================declarations================================================
VIError vi_worker_status_init(
        VIWorkerStatus* const restrict status,
        void* (*worker_fun)(void* arg),
        void* arg
        );

VI_RESULT_TYPE(VIError) vi_worker_status_suspend(VIWorkerStatus* const restrict status)
     VI_ASYNC_SETS_STATE(VIError_None, status);

VI_RESULT_TYPE(VIError) vi_worker_status_resume(VIWorkerStatus* const restrict status)
    VI_ASYNC_SETS_STATE(VIError_None, status);

void vi_worker_status_set_working_mode(
        VIWorkerStatus* const restrict status,
        WorkerStatus wc);

VIError vi_worker_status_self_suspend(void);

WorkerStatus vi_worker_status_get_working_mode(const VIWorkerStatus* const restrict status);

void vi_worker_status_destroy(VIWorkerStatus* const restrict status);



static inline const char* worker_status_to_str(const WorkerStatus wc)
{
    switch (wc)
    {
        case WorkerStatus_Init:             return "Init";
        case WorkerStatus_Working:          return "Working";
        case WorkerStatus_Suspended:        return "Suspended";
        case WorkerStatus_Done:             return "Done";
        case WorkerStatus_Dead:             return "Dead";
        case __WorkerStatus_Count:          assert(0 && "unreachable");
    }

    assert(0 && "unreachable");
}
