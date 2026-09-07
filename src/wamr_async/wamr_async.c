#include "wamr_async.h"
#include "wasm_export.h"

#include <assert.h>

struct __WamrAsyncInternalArg
{
    wasm_module_inst_t module_inst;
    Context* th_ctx;
    Context* parent_ctx;
    WamrAsyncFun user_fun;
    void* user_arg;
};

static Context* parent_ctx;

static void async_fun(void* arg)
{
    struct __WamrAsyncInternalArg* int_arg = arg;

    assert(int_arg);
    assert(int_arg->th_ctx);
    assert(int_arg->parent_ctx);
    assert(int_arg->module_inst);

    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(int_arg->module_inst, STACK_SIZE);

    context_switch(int_arg->th_ctx, int_arg->parent_ctx);

    while(1)
    {
        int_arg->user_fun(exec_env, int_arg->user_arg);
        assert(parent_ctx);
        context_switch(int_arg->th_ctx, parent_ctx);
    }

    wasm_runtime_destroy_exec_env(exec_env);
}

int wamr_async_create(WamrAsync* self, wasm_module_inst_t module_inst, WamrAsyncFun func, void* arg)
{
    int res=0;
    wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, STACK_SIZE);
    Context self_ctx = {0};
    struct __WamrAsyncInternalArg int_arg = {0};

    assert(exec_env);
    assert(self);

    *self = (WamrAsync) {0};
    self->stack = create_new_stack();

    self_ctx = context_self();
    int_arg.th_ctx = &self->ctx;
    int_arg.parent_ctx = &self_ctx;
    int_arg.user_fun = func;
    int_arg.user_arg = arg;
    int_arg.module_inst = module_inst;

    context_init(&self->ctx, &self->stack, async_fun, &int_arg);
    context_switch(&self_ctx, &self->ctx);

    return res;
}

void wamr_async_exec(WamrAsync* self)
{
    Context curr_ctx = context_self();

    parent_ctx = &curr_ctx;
    context_switch(&curr_ctx, &self->ctx);
    parent_ctx = NULL;
}

void wamr_async_destroy(WamrAsync async)
{
    destroy_stack(&async.stack);
}
