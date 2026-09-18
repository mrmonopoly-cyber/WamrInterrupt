#include "dispatcher.h"

#include <unistd.h>

VIError vi_dispatcher_status_init(VIDispatcherStatus* const restrict status,
        void* (*dispatcher_fun) (void* arg),
        void* arg)
{
    assert(status);

    atomic_init(&status->n_requests, 0);
    atomic_init(&status->run, true);
    return vi_worker_status_init(&status->base, dispatcher_fun , arg);
}

void vi_dispatcher_status_destroy(VIDispatcherStatus* const restrict status)
{
    assert(status);

    atomic_store(&status->run, false);
    (void) vi_worker_status_resume(&status->base);

    while( vi_worker_status_get_working_mode(&status->base) != WorkerStatus_Dead )
    {
        usleep(1000);
    }

    vi_worker_status_destroy(&status->base);
}
