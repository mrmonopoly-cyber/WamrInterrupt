#pragma once

#include "base.h"
#include "dispatcher.h"
#include "wasm_export.h"

//================================================types==========================================
typedef struct
{
    VIWorkerStatus base;
    VIDispatcherStatus* p_dispatcher_status;
}VIMainLogicStatus;

typedef struct
{
    wasm_module_inst_t module_inst;
    VIMainLogicStatus* status;
    wasm_function_inst_t main_f;
    atomic_int* out;
}ThMainThreadArg;

//==============================================declarations=====================================
VIError vi_main_logic_init(
        VIMainLogicStatus* const restrict status,
        VIDispatcherStatus* const restrict p_dispatcher_status,
        wasm_function_inst_t f_main,
        wasm_module_inst_t module_inst
        ) VI_RESULT_TYPE;

VIError vi_main_logic_suspend(VIMainLogicStatus* const restrict status) VI_RESULT_TYPE;
VIError vi_main_logic_resume(VIMainLogicStatus* const restrict status) VI_RESULT_TYPE;
WorkerStatus vi_main_logic_get_mode(const VIMainLogicStatus* const restrict status);
void vi_main_logic_destroy(VIMainLogicStatus* const restrict status);

