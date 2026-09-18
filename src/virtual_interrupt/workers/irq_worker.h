#pragma once

#include "dispatcher.h"
#include "../common.h"
#include "../span/span.h"

//================================================types==========================================
typedef wasm_function_inst_t IrqFuncHandler;

typedef struct
{
    VIWorkerStatus base;
    VIDispatcherStatus *p_dispatcher;

    IrqFuncHandler* p_funcs;
    atomic_size_t func_index;

    wasm_exec_env_t th_exec_env;

    atomic_bool run;
}VIIrqWorkerStatus;

typedef struct
{
    wasm_module_inst_t module_inst;
    VIIrqWorkerStatus* status;
    atomic_int* out;
}ThWorkersArg;

typedef SPAN_TEMPLATE(VIIrqWorkerStatus) VISpanWorkerStatus;

//==============================================declarations======================================
void vi_irq_worker_destroy(VIIrqWorkerStatus* const restrict status);

VIError vi_irq_worker_init(
        VIIrqWorkerStatus* const restrict status,
        VIDispatcherStatus* const restrict p_status_dispatcher,
        IrqFuncHandler* p_funcs,
        wasm_module_inst_t module_inst
        ) VI_RESULT_TYPE;
WorkerStatus vi_irq_worker_get_mode(VIIrqWorkerStatus* const restrict status);
VIError vi_irq_worker_suspend(VIIrqWorkerStatus* const restrict status) VI_RESULT_TYPE;
VIError vi_irq_worker_resume(VIIrqWorkerStatus* const restrict status) VI_RESULT_TYPE;
void vi_irq_worker_destroy(VIIrqWorkerStatus* const restrict status);
