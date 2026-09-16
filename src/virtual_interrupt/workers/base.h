#pragma once

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

typedef struct
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

VIError vi_worker_status_self_suspend(void);
void vi_worker_status_suspend(VIWorkerStatus* const restrict status);
void vi_worker_status_resume(VIWorkerStatus* const restrict status);

void vi_worker_status_set_working_mode(
        VIWorkerStatus* const restrict status,
        WorkerStatus wc);
WorkerStatus vi_worker_status_get_working_mode( VIWorkerStatus* const restrict status);

VIError vi_worker_status_signal(VIWorkerStatus* const restrict status) VI_RESULT_TYPE;
VIError vi_worker_status_signal(VIWorkerStatus* const restrict status) VI_RESULT_TYPE;

void vi_worker_status_destroy(VIWorkerStatus* const restrict status);
