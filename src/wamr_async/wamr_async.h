#pragma once

#include "../ctx_switch/ctx_switch.h"
#include "wasm_export.h"

typedef UserFunc WamrAsyncFun;

typedef struct
{
    Stack stack;
    Context ctx;
    wasm_exec_env_t exec_env;
}WamrAsync;


WamrAsync wamr_async_create(wasm_module_inst_t module_inst, WamrAsyncFun func, void* arg);

void wamr_async_destroy(WamrAsync async);
