#pragma once

#include "../ctx_switch/ctx_switch.h"
#include "wasm_export.h"

typedef void (*WamrAsyncFun) (wasm_exec_env_t exec_env, void* user_arg);

typedef struct
{
    Stack stack;
    Context ctx;
}WamrAsync;

int wamr_async_create(WamrAsync* self, wasm_module_inst_t module_inst, WamrAsyncFun func, void* arg);

void wamr_async_exec(WamrAsync* self);

void wamr_async_destroy(WamrAsync async);
