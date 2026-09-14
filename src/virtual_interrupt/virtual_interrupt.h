#pragma once

#include <assert.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "minheap/minheap.h"
#include "span/span.h"
#include "wasm_export.h"
#include "spscq/spscq.h"
#include "workers/workers.h"
#include "common.h"


typedef size_t IrqLine;
typedef size_t ThreadID;
typedef TEMPLATE_SPSCQ(IrqLine, READY_QUEUE_CAP) SPSCQ_UReq;
typedef MINHEAP_TEMPLATE(IrqLine, WAIT_QUEUE_CAP) MinheapUReq;

typedef struct __VirtualInterruptDispatcher
{
    VIMainLogicStatus main_fun_status;
    VISpanWorkerStatus workers;
    VIDispatcherStatus dispatcher;

    IrqFuncHandler* funcs;
    size_t n_funcs;

    SPSCQ_UReq channel_ready_ureq;
    MinheapUReq minheap_ureq;

    size_t executing_worker; //INFO: 0 means None, K means workers[k-1] IS CURRENTLY EXECUTING

    wasm_module_inst_t module_inst;
}VIDispatcher;

typedef struct
{
    int suspend_signal;
    int resume_signal;
    size_t depth;
}VIDispatcherConf;

VIError vidispatcher_init_full(
        VIDispatcher* const restrict dispatcher,
        wasm_module_inst_t module_inst,
        wasm_function_inst_t main_f,
        const size_t n_lines,
        const VIDispatcherConf conf);

static inline VIError vidispatcher_init(
        VIDispatcher* const restrict dispatcher,
        wasm_module_inst_t module_inst,
        wasm_function_inst_t main_f,
        const size_t n_lines
        )
{
    const VIDispatcherConf default_conf =
    {
        .depth = 8,
        .suspend_signal = VI_DEFAULT_SIG_SUSPEND,
        .resume_signal = VI_DEFAULT_SIG_RESUME,
    };
    return vidispatcher_init_full(dispatcher, module_inst, main_f, n_lines, default_conf);
}

VIError vidispatcher_assign_irq_to_line(
        VIDispatcher* const restrict dispatcher,
        const IrqFuncHandler irq_handler,
        const size_t line);

VIError vidispatcher_start(VIDispatcher* const restrict dispatcher);

VIError vidispatcher_trigger_interrupt(VIDispatcher* const restrict dispatcher, const IrqLine line);

void vidispatcher_destroy(VIDispatcher* const restrict dispatcher);

static inline const char* vi_error_to_str(const VIError err)
{
    extern int VI_ERROR_ERRNO;
    switch (err)
    {
        case VIError_None:                  return "";
        case VIError_InvalidInput:          return "invalid input";
        case VIError_Queue:                 return "Internal Queue error: Full?";
        case VIError_WAMR:                  return "wamr error";
        case VIError_Libc:                  return "libc error";
    }

    assert(0 && "unreachable");
}
