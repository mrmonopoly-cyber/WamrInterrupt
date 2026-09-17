#pragma once

#include "workers/irq_worker.h"

#define FOR_EACH_IRQ_WORKER_INDEX(NAME, WORKERS) \
    for (size_t NAME = 1; i <= (span_len(WORKERS)); i++)

#define FOR_EACH_IRQ_WORKER_RANGE(NAME, MAX) \
    for (size_t NAME = 1; i <= (MAX); i++)

static inline VIIrqWorkerStatus* _get_worker(
        const VISpanWorkerStatus* const restrict list,
        const size_t i)
{
    VIIrqWorkerStatus* res = NULL;
    SpanError span_res;

    assert(list);

    if ( i > 0 && i <= span_len(list) )
    {
        span_get(list, i - 1, &res, &span_res);

        assert( span_res == SpanError_None );
    }

    return res;
}
