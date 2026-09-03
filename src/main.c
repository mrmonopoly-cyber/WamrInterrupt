#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "wasm_export.h"
#include "bh_read_file.h"

static char error_buf[256] = {0};

#define ArraySize(ARR) (sizeof(ARR)/sizeof(ARR[0]))
#define INPUT_ERROR_BUF (error_buf), ArraySize((error_buf))
#define GOTO_END goto end
#define GOTO_END_AND_CUSTOM_ERROR(...)                  \
    do{                                                 \
        const char* msg[] = {__VA_ARGS__};              \
        if(ArraySize(msg))                              \
        {                                               \
            snprintf(INPUT_ERROR_BUF, "%s", msg[0]);    \
        }                                               \
        GOTO_END;                                       \
    }while(0);

void host_stdout_print(wasm_exec_env_t env, const char * msg)
{
    (void) env;
    printf("board print basic: %s\n", msg);
}

extern void new_host_stdout_print(wasm_exec_env_t env, const char * msg)
{
    (void) env;
    printf("board print advanced: %s\n", msg);
}

int main(int argc, char *argv[])
{
    const char* input_file = argv[1];
    const uint32_t stack_size = 65536, heap_size = 1 << 20;

    wasm_module_t module = {0};
    wasm_module_inst_t module_inst = {0};
    wasm_function_inst_t board_main_f = {0};
    wasm_exec_env_t main_exec_env = {0};

    char* buf = NULL;
    uint32_t file_buffer_size = 0;

    if ( argc < 2)
    {
        fprintf(stderr, "missing input file: *.aot\n");
        return 1;
    }

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

    wasm_runtime_init();

    if( !wasm_runtime_register_natives("env", native_symbols, ArraySize(native_symbols)) )
    {
        GOTO_END_AND_CUSTOM_ERROR("failed registering native_symbols");
    }

    buf = bh_read_file_to_buffer(input_file, &file_buffer_size);
    if ( !buf ) GOTO_END_AND_CUSTOM_ERROR("failed loading input file");

    module = wasm_runtime_load((uint8_t *) buf, file_buffer_size, INPUT_ERROR_BUF);
    if ( !module ) GOTO_END;

    module_inst = wasm_runtime_instantiate(module, stack_size, heap_size, INPUT_ERROR_BUF);
    if ( !module_inst ) GOTO_END;

    board_main_f = wasm_runtime_lookup_function(module_inst, "board_main");
    if ( !board_main_f ) GOTO_END_AND_CUSTOM_ERROR("board main function not found");

    main_exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);

    if ( !wasm_runtime_call_wasm(main_exec_env, board_main_f, 0, NULL) )
    {
        GOTO_END_AND_CUSTOM_ERROR(wasm_runtime_get_exception(module_inst));
    }


    printf("done\n");

end:
    if( strcmp(error_buf, "") ) fprintf(stderr, "wamr error: %s\n", error_buf);

    if ( module_inst ) wasm_runtime_terminate(module_inst);

    if ( main_exec_env ) wasm_runtime_destroy_exec_env(main_exec_env);

    if ( module_inst )
    {
        wasm_runtime_deinstantiate(module_inst);
    }
    if ( module ) wasm_runtime_unload(module);

    wasm_runtime_destroy();

    if ( buf ) free(buf);
    return 0;
}

