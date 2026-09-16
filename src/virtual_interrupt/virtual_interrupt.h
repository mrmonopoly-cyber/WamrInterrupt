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

#ifndef READY_QUEUE_CAP
#define READY_QUEUE_CAP 32
#endif

#ifndef WAIT_QUEUE_CAP
#define WAIT_QUEUE_CAP 32
#endif

#ifndef VI_DEFAULT_DEPTH
#define VI_DEFAULT_DEPTH 8
#endif // !VI_DEFAULT_DEPTH

#ifndef VI_DEFAULT_LOG_FILE_BASE_PATH
#define VI_DEFAULT_LOG_FILE_BASE_PATH "vi_log"
#endif // !VI_DEFAULT_LOG_FILE_BASE_PATH

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
    const char* log_file_base_path;
}VIDispatcherConf;

#define VIDISPATCHERCONF_DEFUALT                                        \
    {                                                                   \
        .depth = VI_DEFAULT_DEPTH,                                      \
        .suspend_signal = VI_DEFAULT_SIG_SUSPEND,                       \
        .resume_signal = VI_DEFAULT_SIG_RESUME,                         \
        .log_file_base_path = VI_DEFAULT_LOG_FILE_BASE_PATH,            \
    }


VIError vidispatcher_init_full(
        VIDispatcher* const restrict dispatcher,
        wasm_module_inst_t module_inst,
        wasm_function_inst_t main_f,
        const size_t n_lines,
        const VIDispatcherConf conf) VI_RESULT_TYPE;

VIError vidispatcher_init(
        VIDispatcher* const restrict dispatcher,
        wasm_module_inst_t module_inst,
        wasm_function_inst_t main_f,
        const size_t n_lines
        ) VI_RESULT_TYPE;

VIError vidispatcher_assign_irq_to_line(
        VIDispatcher* const restrict dispatcher,
        const IrqFuncHandler irq_handler,
        const size_t line) VI_RESULT_TYPE;

VIError vidispatcher_start(VIDispatcher* const restrict dispatcher) VI_RESULT_TYPE;

VIError vidispatcher_trigger_interrupt(VIDispatcher* const restrict dispatcher, const IrqLine line) VI_RESULT_TYPE;

void vidispatcher_destroy(VIDispatcher* const restrict dispatcher);

const char* vidispatcher_error_to_str(const VIError err);
