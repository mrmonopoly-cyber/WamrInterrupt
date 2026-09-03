#include "wamr_async.h"
#include "wasm_export.h"
#include <assert.h>

WamrAsync wamr_async_create(wasm_module_inst_t module_inst, WamrAsyncFun func, void* arg)
{
    Stack stack = create_new_stack();
    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, STACK_SIZE);
    Context ctx = {0};

    assert(exec_env);

    context_init(&ctx, &stack, func, arg);

    return (WamrAsync){
        .stack = stack,
        .ctx = ctx,
        .exec_env = exec_env,
    };
}

void wamr_async_destroy(WamrAsync async)
{
    destroy_stack(&async.stack);
    wasm_runtime_destroy_exec_env(async.exec_env);
}
