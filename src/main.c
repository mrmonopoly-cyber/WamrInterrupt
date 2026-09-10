#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "virtual_interrupt/virtual_interrupt.h"
#include "wasm_export.h"
#include "bh_read_file.h"

static char error_buf[256] = {0};

#define ArraySize(ARR) (sizeof(ARR)/sizeof(ARR[0]))
#define INPUT_ERROR_BUF (error_buf), ArraySize((error_buf))
#define GOTO_END goto end
#define GOTO_END_AND_CUSTOM_ERROR_LINE(LINE, ...)                       \
    do{                                                                 \
        const char* msg[] = {__VA_ARGS__};                              \
        if(ArraySize(msg))                                              \
        {                                                               \
            snprintf(INPUT_ERROR_BUF, "line: %d: %s", (LINE), msg[0]);  \
        }                                                               \
        GOTO_END;                                                       \
    }while(0);

#define GOTO_END_AND_CUSTOM_ERROR(...) GOTO_END_AND_CUSTOM_ERROR_LINE(__LINE__, __VA_ARGS__) 

typedef struct
{
    wasm_exec_env_t* main_exec_env;
}PthreadFuncArg;

typedef struct
{
    wasm_function_inst_t func;
    wasm_module_inst_t module_inst;
}ThHandlerArg;


void host_stdout_print(wasm_exec_env_t env, const char * msg)
{
    (void) env;
    printf("board print basic: %s\n", msg);
}

void new_host_stdout_print(wasm_exec_env_t env, const char * msg)
{
    (void) env;
    printf("board print advanced: %s\n", msg);
}

int main(int argc, char *argv[])
{
    const char* input_file = argv[1];
    const uint32_t stack_size = 65536, heap_size = 1 << 20;
    const char* irq_functions[] =
    {
        "led_value_i1",
        "led_value_i2",
    };

    char* buf = NULL;
    uint32_t file_buffer_size = 0;
    bool init_wamr_ok = false;

    VIDispatcher vi_dispatcher = {0};
    VIError vi_err = {0};

    wasm_module_t module = {};
    wasm_module_inst_t module_inst = {};
    wasm_exec_env_t main_exec_env = {};
    wasm_function_inst_t main_func = {};

    NativeSymbol native_symbols[] =
    {
        {
            .symbol ="host_stdout_print",
            .func_ptr = (void*) host_stdout_print,
            .signature = "($)",
            .attachment = NULL,
        },

        {
            .symbol ="new_host_stdout_print",
            .func_ptr = (void*) new_host_stdout_print,
            .signature = "($)",
            .attachment = NULL,
        }
    };

    //===============================================init=========================================

    if ( argc < 2)
    {
        fprintf(stderr, "missing input file: *.aot\n");
        return 1;
    }

    if ( !(init_wamr_ok = wasm_runtime_init()) )
    {
        GOTO_END_AND_CUSTOM_ERROR("failed wasm runtime init");
    }

    if( !wasm_runtime_register_natives("env", native_symbols, ArraySize(native_symbols)) )
    {
        GOTO_END_AND_CUSTOM_ERROR("failed registering native_symbols");
    }

    if ( !(buf = bh_read_file_to_buffer(input_file, &file_buffer_size)) )
    {
        GOTO_END_AND_CUSTOM_ERROR("failed loading input file");
    }

    if ( !(module = wasm_runtime_load((uint8_t *) buf, file_buffer_size, INPUT_ERROR_BUF)) )
    {
        GOTO_END;
    }

    if ( !(module_inst = wasm_runtime_instantiate(module, stack_size, heap_size, INPUT_ERROR_BUF)) )
    {
        GOTO_END;
    }

    if ( !(main_exec_env = wasm_runtime_create_exec_env(module_inst, stack_size)) )
    {
        GOTO_END_AND_CUSTOM_ERROR("failed creating main_exec_env");
    }

    if ( !(main_func = wasm_runtime_lookup_function(module_inst, "board_main")) )
    {
        GOTO_END_AND_CUSTOM_ERROR("failed loading board main");
    }

    if ( (vi_err = vidispatcher_init(&vi_dispatcher, module_inst, main_func,  2, 1)) )
    {
        GOTO_END_AND_CUSTOM_ERROR(vi_error_to_str(vi_err));
    }

    for (size_t i=0; i<ArraySize(irq_functions); i++)
    {
        wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, irq_functions[i]);
        if ( (vi_err = vidispatcher_assign_irq_to_line(&vi_dispatcher, func, i)) )
        {
            GOTO_END_AND_CUSTOM_ERROR(vi_error_to_str(vi_err));
        }
    }

    if ( (vi_err = vidispatcher_start(&vi_dispatcher)) )
    {
        GOTO_END_AND_CUSTOM_ERROR(vi_error_to_str(vi_err));
    }

    //========================================fantastic logic=====================================

    printf("normal execution\n");
    usleep(3 * 1000 * 1000);

    printf("triggering interrupt in ascencing priority\n");

    if ( (vi_err = vidispatcher_trigger_interrupt(&vi_dispatcher, 0)) )
    {
        GOTO_END_AND_CUSTOM_ERROR(vi_error_to_str(vi_err));
    }

    printf("normal execution\n");
    usleep(2 * 1000 * 1000);

    if ( (vi_err = vidispatcher_trigger_interrupt(&vi_dispatcher, 1)) )
    {
        GOTO_END_AND_CUSTOM_ERROR(vi_error_to_str(vi_err));
    }

    printf("normal execution\n");
    usleep(10 * 1000 * 1000);

    printf("triggering interrupt in descencing priority\n");
    if ( (vi_err = vidispatcher_trigger_interrupt(&vi_dispatcher, 1)) )
    {
        GOTO_END_AND_CUSTOM_ERROR(vi_error_to_str(vi_err));
    }

    printf("normal execution\n");
    usleep(2 * 1000 * 1000);

    if ( (vi_err = vidispatcher_trigger_interrupt(&vi_dispatcher, 0)) )
    {
        GOTO_END_AND_CUSTOM_ERROR(vi_error_to_str(vi_err));
    }

    printf("normal execution\n");
    usleep(10 * 1000 * 1000);

    //========================================stopping thread=====================================
    printf("cancelling thread\n");

    printf("done\n");

end:
    if( strcmp(error_buf, "") ) fprintf(stderr, "wamr error: %s\n", error_buf);

    vidispatcher_destroy(&vi_dispatcher);

    if ( module_inst )          wasm_runtime_terminate(module_inst);
    if ( main_exec_env )        wasm_runtime_destroy_exec_env(main_exec_env);
    if ( module_inst )          wasm_runtime_deinstantiate(module_inst);
    if ( module )               wasm_runtime_unload(module);
    if ( init_wamr_ok )         wasm_runtime_destroy();

    if ( buf ) free(buf);
    return 0;
}

