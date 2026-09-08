#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "virtual_interrupt/virtual_interrupt.h"
#include "wasm_export.h"
#include "bh_read_file.h"

static char error_buf[256] = {0};

static pthread_mutex_t sigint_mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sigint_cond = PTHREAD_COND_INITIALIZER;

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

void sigint_handler(int signal)
{
    (void) signal;
    printf("sigint handler: wait\n");

    pthread_mutex_lock(&sigint_mut);
    pthread_cond_wait(&sigint_cond, &sigint_mut);
    pthread_mutex_unlock(&sigint_mut);

    printf("sigint handler: done\n");
}

void th_handler(wasm_exec_env_t exec_en, void* arg)
{
    ThHandlerArg* th_arg = arg;
    if ( !wasm_runtime_call_wasm(exec_en, th_arg->func, 0, NULL) )
    {
        fprintf(stderr, "th_handler error: %s\n", wasm_runtime_get_exception(th_arg->module_inst));
        GOTO_END_AND_CUSTOM_ERROR(wasm_runtime_get_exception(th_arg->module_inst));
    }

end:
    printf("th_handler: terminating\n");
    return;
}

void worker_thread_cleanup(void* arg)
{
    wasm_exec_env_t* th_exec_env = arg;

    if ( *th_exec_env ) wasm_runtime_destroy_exec_env(*th_exec_env);
    wasm_runtime_destroy_thread_env();
}

void* worker_thread(void* arg)
{
    uintptr_t res = 1;
    PthreadFuncArg func_arg = *(PthreadFuncArg*) arg;
    wasm_exec_env_t th_exec_env = {};
    wasm_module_inst_t module_inst = {};
    wasm_function_inst_t main_func = {};
    sigset_t set = {0};
    int err;

    //====================================init=================================================

    pthread_cleanup_push(worker_thread_cleanup, &th_exec_env);

    sigemptyset(&set);
    sigaddset(&set, SIGPOLL);

    if ( (err = pthread_sigmask(SIG_UNBLOCK, &set, NULL)) )
    {
        GOTO_END_AND_CUSTOM_ERROR(strerror(err));
    }

    wasm_runtime_init_thread_env();

    if ( !wasm_runtime_thread_env_inited() )
    {
        GOTO_END_AND_CUSTOM_ERROR("thread env not inited");
    }

    if ( !(th_exec_env = wasm_runtime_spawn_exec_env(*func_arg.main_exec_env)) )
    {
        GOTO_END_AND_CUSTOM_ERROR("failed creating thread exec env");
    }

    if ( !(module_inst = wasm_runtime_get_module_inst(*func_arg.main_exec_env)) )
    {
        GOTO_END_AND_CUSTOM_ERROR("failed fetching module instance");
    }
    
    if ( !(main_func = wasm_runtime_lookup_function(module_inst, "board_main")) )
    {
        GOTO_END_AND_CUSTOM_ERROR("failed loading board main");
    }

    //====================================main logic============================================

    if ( wasm_runtime_call_wasm(th_exec_env, main_func, 0, NULL) )
    {
        GOTO_END_AND_CUSTOM_ERROR(wasm_runtime_get_exception(module_inst));
    }

    //====================================end===================================================
    res =0;
end:
    pthread_cleanup_pop(1);
    return (void*) res;
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
    void* th_res = NULL;
    uint32_t file_buffer_size = 0;
    uintptr_t err;
    bool init_wamr_ok = false;

    VIDispatcher vi_dispatcher = {0};
    VIError vi_err = {0};

    wasm_module_t module = {};
    wasm_module_inst_t module_inst = {};
    wasm_exec_env_t main_exec_env = {};

    pthread_t th_id_workder;

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

    PthreadFuncArg func_arg = 
    {
        .main_exec_env = &main_exec_env,
    };

    struct sigaction sa = {};

    sigemptyset(&sa.sa_mask);
    sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);
    sa.sa_handler = sigint_handler;

    //===============================================init=========================================

    if ( argc < 2)
    {
        fprintf(stderr, "missing input file: *.aot\n");
        return 1;
    }

    if ( sigaction(SIGPOLL, &sa, NULL) )
    {
        GOTO_END_AND_CUSTOM_ERROR(strerror(errno));
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

    if( (err = pthread_create(&th_id_workder, NULL, worker_thread, &func_arg)) )
    {
        GOTO_END_AND_CUSTOM_ERROR(strerror(err));
    }

    if ( (vi_err = vidispatcher_init(&vi_dispatcher, module_inst, 2, 4)) )
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
    sleep(2);

    //========================================stopping thread=====================================
    printf("cancelling thread\n");

    if ( (err =  pthread_cancel(th_id_workder)) )
    {
        GOTO_END_AND_CUSTOM_ERROR(strerror(err));
    }

    pthread_join(th_id_workder, &th_res);
    if( th_res != PTHREAD_CANCELED )
    {
        GOTO_END_AND_CUSTOM_ERROR("invalid pthread_join res");
    }


    printf("done\n");

end:
    if( strcmp(error_buf, "") ) fprintf(stderr, "wamr error: %s\n", error_buf);

    if ( module_inst )          wasm_runtime_terminate(module_inst);
    if ( main_exec_env )        wasm_runtime_destroy_exec_env(main_exec_env);
    if ( module_inst )          wasm_runtime_deinstantiate(module_inst);
    if ( module )               wasm_runtime_unload(module);
    if ( init_wamr_ok )         wasm_runtime_destroy();

    if ( buf ) free(buf);
    return 0;
}

