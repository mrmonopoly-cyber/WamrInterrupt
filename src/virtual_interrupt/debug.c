#include "common.h"
#include "virtual_interrupt.h"
#include "workers/base.h"
#include "workers/irq_worker.h"
#include "irq_workers_list.h"

#include <errno.h>
#include <stdlib.h>

#define log(...) vi_log(VILoggerLevel_Debug, log_buffer, sizeof(log_buffer), "VIDebug", __VA_ARGS__)
#define log_warn(...) vi_log(VILoggerLevel_Warning, log_buffer, sizeof(log_buffer), "VIDebug", __VA_ARGS__)
#define log_err(...) vi_log(VILoggerLevel_Error, log_buffer, sizeof(log_buffer),"VIDebug",  __VA_ARGS__)

VIError debug_vidispatcher_get_workers_status(
        const VIDispatcher* const restrict dispatcher,
        VIDWorkerStatus** out, size_t* n_out)
{
    VIDWorkerStatus* o_buffer;
    VIIrqWorkerStatus* irq_worker;
    if ( !dispatcher || !out || !n_out || *out ) return VIError_InvalidInput;

    const size_t n_irq_workers =span_len(&dispatcher->workers);
    const size_t n_workers = 2 + n_irq_workers;

    char log_buffer[64] = {0};

    *out = malloc(n_workers * sizeof(**out));

    if ( !*out )
    {
        _vi_set_errno(errno);
        return VIError_Libc;
    }

    o_buffer = *out;

    o_buffer[0] = (VIDWorkerStatus){
        .t = VIDWorkerID_Dispatcher,
        .status = vi_worker_status_get_working_mode(&dispatcher->dispatcher.base),
    };

    o_buffer[1] = (VIDWorkerStatus){
        .t = VIDWorkerID_Main,
        .status = vi_main_logic_get_mode(&dispatcher->main_fun_status),
    };

    FOR_EACH_IRQ_WORKER_INDEX(i, &dispatcher->workers)
    {
        irq_worker = _get_worker(&dispatcher->workers, i);
        if ( irq_worker )
        {
            o_buffer[2 + ( i - 1) ] = (VIDWorkerStatus){ //i starts from 1
                .t = VIDWorkerID_Irq,
                .status = vi_irq_worker_get_mode(irq_worker),
            };
        }
        else
        {
            log_err("failed retriening info about irq worker %zu", i);
        }
    }

    *n_out = n_workers;

    return VIError_None;
}
