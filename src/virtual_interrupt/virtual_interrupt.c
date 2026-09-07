#include "virtual_interrupt.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

#include "spscq/spscq.h"
#include "wasm_export.h"

typedef struct
{
    wasm_module_inst_t module_inst;
    struct VIWorkerStatus* status;
    int out;
}ThWorkersArg;

static void* _th_irq_worker(void* arg);
static void* _th_dispatcher(void* arg);

static void _th_irq_worker_cleanup(void* arg);
static void _th_irq_worker_signal_handler(int signal, siginfo_t* info, void* ctx);
static void __default_irq_handler(void);

static inline struct VIWorkerStatus* get_active_worker(const VIDispatcher* const restrict d);

VIError vidispatcher_init(
        VIDispatcher* const restrict dispatcher,
        wasm_module_inst_t module_inst,
        const size_t n_lines,
        const size_t depth)
{
    VIError res = VIError_None;
    struct VIWorkerStatus* workers = NULL;
    IrqFuncHandler* funcs = NULL;
    size_t workers_ok=0;

    if( !dispatcher || !module_inst ||  !depth )
    {
        res = VIError_InvalidInput;
        goto end;
    }

    workers = malloc(depth * sizeof(*workers));
    funcs = malloc(n_lines * sizeof(*funcs));

    if ( !workers || !funcs )
    {
        res = VIError_Libc;
        goto end;
    }

    for(size_t i=0; i<n_lines; i++)
    {
        funcs[i] = __default_irq_handler;
    }

    struct sigaction sa ={0};
    sa.sa_sigaction = _th_irq_worker_signal_handler;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIG_PREEMPTION_WORKERS);

    if ( sigaction(SIG_PREEMPTION_WORKERS, &sa, NULL) < 0 )
    {
        res =VIError_Libc;
        goto end;
    }

    if ( sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL) < 0 )
    {
        res =VIError_Libc;
        goto end;
    }

    for(size_t i=0; i<depth; i++)
    {
        int err;
        struct VIWorkerStatus* status = &workers[i];
        ThWorkersArg arg = {
            .module_inst = module_inst,
            .status = status,
            .out = -1};

        pthread_cond_init(&status->data_cond, NULL);
        pthread_mutex_init(&status->data_mutex, NULL);

        pthread_cond_init(&status->preemption_cond, NULL);
        pthread_mutex_init(&status->preemption_mutex, NULL);

        status->func = __default_irq_handler;
        status->p_dispatcher_cond = &dispatcher->dispatcher_cond;
        status->p_dispatcher_mutex = &dispatcher->dispatcher_mutex;
        atomic_init(&status->working, false);

        if( ( err = pthread_create(&status->tid, NULL, _th_irq_worker, &arg) ) )
        {
            errno = err;
            res = VIError_Libc;
            goto end;
        }

        //spinlock
        while(arg.out == -1);

        if (arg.out != VIError_None)
        {
            res = VIError_WAMR;
            goto end;
        }

        workers_ok++;
    }

    dispatcher->n_workers = depth;
    dispatcher->workers = workers;

    dispatcher->n_funcs = n_lines;
    dispatcher->funcs = funcs;

    spscq_init(&dispatcher->channel_ureq);

    pthread_cond_init(&dispatcher->dispatcher_cond, NULL);
    pthread_mutex_init(&dispatcher->dispatcher_mutex, NULL);

end:
    for(size_t i=0; i<workers_ok; i++)
    {
        struct VIWorkerStatus *worker = &dispatcher->workers[i];

        pthread_cancel(worker->tid);
        pthread_join(worker->tid, NULL);

        pthread_mutex_destroy(&worker->data_mutex);
        pthread_cond_destroy(&worker->data_cond);

        pthread_mutex_destroy(&worker->preemption_mutex);
        pthread_cond_destroy(&worker->preemption_cond);
    }

    pthread_mutex_destroy(&dispatcher->dispatcher_mutex);
    pthread_cond_destroy(&dispatcher->dispatcher_cond);
    if (workers) free(workers);
    if (funcs) free(funcs);
    return res;
}

VIError vidispatcher_assign_irq_to_line(
        VIDispatcher* const restrict dispatcher,
        const IrqFuncHandler irq_handler,
        const size_t line)
{
    VIError res = VIError_None;

    if( !dispatcher || line < dispatcher->n_funcs)
    {
        res = VIError_InvalidInput;
        goto end;
    }

    dispatcher->funcs[line] = irq_handler;


end:
    return res;
}

VIError vidispatcher_start(VIDispatcher* const restrict dispatcher)
{
    VIError res = VIError_None;

    if ( !dispatcher )
    {
        res = VIError_InvalidInput;
        goto end;
    }

    if( pthread_create(&dispatcher->dispatcher_tid, NULL, _th_dispatcher , dispatcher) < 0 )
    {
        res = VIError_Libc;
        goto end;
    }

end:
    return res;
}

void vidispatcher_destroy(VIDispatcher* const restrict dispatcher)
{
    if(dispatcher)
    {
        pthread_cancel(dispatcher->dispatcher_tid);
        pthread_join(dispatcher->dispatcher_tid, NULL);

        for(size_t i=0; i<dispatcher->n_workers; i++)
        {
            struct VIWorkerStatus *worker = &dispatcher->workers[i];

            pthread_cancel(worker->tid);
            pthread_join(worker->tid, NULL);

            pthread_mutex_destroy(&worker->data_mutex);
            pthread_cond_destroy(&worker->data_cond);

            pthread_mutex_destroy(&worker->preemption_mutex);
            pthread_cond_destroy(&worker->preemption_cond);
        }

        pthread_mutex_destroy(&dispatcher->dispatcher_mutex);
        pthread_cond_destroy(&dispatcher->dispatcher_cond);
        if ( dispatcher->workers ) free(dispatcher->workers);
        if ( dispatcher->funcs ) free(dispatcher->funcs);
        
        *dispatcher = (VIDispatcher) {};
    }
}

static void _th_irq_worker_cleanup(void* arg)
{
    wasm_exec_env_t* th_exec_env = arg;
    if ( th_exec_env && *th_exec_env ) wasm_runtime_destroy_exec_env(*th_exec_env);
    wasm_runtime_destroy_thread_env();
}

static void* _th_irq_worker(void* arg)
{
    uintptr_t res = VIError_None;
    ThWorkersArg* th_arg = arg;

    struct VIWorkerStatus* status = th_arg->status;
    wasm_module_inst_t module_inst = th_arg->module_inst;
    wasm_exec_env_t th_exec_env = {};
    sigset_t set = {};
    int err;

    assert(module_inst);

    pthread_cleanup_push(_th_irq_worker_cleanup, &th_exec_env);

    wasm_runtime_init_thread_env();
    th_exec_env = wasm_runtime_create_exec_env(module_inst, 16 << 10); //16 KB
    if ( !th_exec_env )
    {
        res = VIError_WAMR;
        goto end;
    }

    sigemptyset(&set);
    sigaddset(&set, SIG_PREEMPTION_WORKERS);
    if ( ( err =pthread_sigmask(SIG_UNBLOCK, &set, NULL) ) < 0 )
    {
        res = VIError_Libc;
        errno = err;
        goto end;
    }

    while(1)
    {
        pthread_testcancel();

        pthread_mutex_lock(&status->data_mutex);
        pthread_cond_wait(&status->data_cond, &status->data_mutex);

        atomic_store(&status->working, true);
        status->func();
        atomic_store(&status->working, false);

        pthread_mutex_unlock(&status->data_mutex);
    }

end:
    pthread_cleanup_pop(true);
    return (void*) res;
}

static void* _th_dispatcher(void* arg)
{
    uintptr_t res = VIError_None;
    VIDispatcher* dispatcher = arg;
    int32_t ureq = -1;
    struct VIWorkerStatus* old_worker, *new_worker;
    SPSCQ_UReq* c_ureq = &dispatcher->channel_ureq;

    assert(dispatcher);

    while( 1 )
    {
        bool worker_finish  = false;

        old_worker = get_active_worker(dispatcher);

        if(
                old_worker->working &&
                atomic_load(&c_ureq->write) != atomic_load(&c_ureq->read)
          )
        {
            pthread_mutex_lock(&dispatcher->dispatcher_mutex);
            pthread_cond_wait(&dispatcher->dispatcher_cond, &dispatcher->dispatcher_mutex);
            pthread_mutex_unlock(&dispatcher->dispatcher_mutex);
        }

        old_worker = get_active_worker(dispatcher);

        while( dispatcher->executing_worker )
        {
            if( !old_worker->working )
            {
                dispatcher->executing_worker--;
                worker_finish  = true;
            }
            else if( worker_finish ) //resume stopped worker
            {
                pthread_mutex_lock(&old_worker->preemption_mutex);
                pthread_cond_signal(&old_worker->preemption_cond);
                pthread_mutex_unlock(&old_worker->preemption_mutex);
                break;
            }
            else
            {
                break;
            }
        }

        //TODO: priority
        spscq_pop(&dispatcher->channel_ureq, &ureq);
        if( ureq != -1 )
        {
            old_worker = get_active_worker(dispatcher);
            dispatcher->executing_worker++;
            new_worker = get_active_worker(dispatcher);
            new_worker->func = dispatcher->funcs[ureq];

            assert(new_worker->func);

            //stop current worker (i)
            pthread_mutex_lock(&old_worker->preemption_mutex);
            pthread_cond_signal(&old_worker->preemption_cond);
            pthread_mutex_unlock(&old_worker->preemption_mutex);

            //start new thread (i+1)
            pthread_mutex_lock(&new_worker->preemption_mutex);
            pthread_cond_signal(&new_worker->preemption_cond);
            pthread_mutex_unlock(&new_worker->preemption_mutex);
        }
        else
        {
            //TODO: waiting queue?
        }
        ureq = -1;
    }

    return (void*) res;
}

static inline struct VIWorkerStatus* get_active_worker(const VIDispatcher* const restrict d)
{
    return &d->workers[d->executing_worker - 1];
}

static void _th_irq_worker_signal_handler(int signal, siginfo_t* info, void* ctx)
{
    assert(signal == SIG_PREEMPTION_WORKERS);
    (void) ctx;

    struct VIWorkerStatus* status = info->si_value.sival_ptr;

    pthread_mutex_lock(&status->preemption_mutex);
    pthread_cond_wait(&status->preemption_cond, &status->preemption_mutex);
    pthread_mutex_unlock(&status->preemption_mutex);
}

static void __default_irq_handler(void)
{
    /*does noting*/
}
