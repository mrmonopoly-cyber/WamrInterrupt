#include "virtual_interrupt.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "minheap/minheap.h"
#define SPAN_IMPLEMENTATION
#include "span/span.h"
#include "spscq/spscq.h"
#include "wasm_export.h"

typedef struct
{
    wasm_module_inst_t module_inst;
    VIWorkerStatus* status;
    int out;
}ThWorkersArg;

typedef struct
{
    wasm_module_inst_t module_inst;
    struct VIMainFunStatus* status;
    wasm_function_inst_t main_f;
    int* out;
}ThMainThreadArg;

static void* _th_main_thread(void* arg);
static void* _th_irq_worker(void* arg);
static void* _th_dispatcher(void* arg);

static void _th_irq_worker_cleanup(void* arg);
static void _th_irq_worker_signal_handler(int signal, siginfo_t* info, void* ctx);

static inline void _suspend_main_thread(struct VIMainFunStatus* const restrict main_thread);

static inline void _preempt_thread(VIPreemptionStatus* const restrict main_f);
static inline void _resume_thread_preemption(VIPreemptionStatus* const restrict status);
static inline void _destroy_preemption_status(VIPreemptionStatus* const restrict status);
static inline void _destroy_signal_status(VIDispatcherSignalStatus* const restrict status);

static inline void _dispatcher_signal(VIDispatcherSignalStatus* status);

static inline int _init_preemption_status(VIPreemptionStatus* status);
static inline void _preemption_status_signal(VIPreemptionStatus* status);
static inline void _preemption_status_wait(VIPreemptionStatus* status);

static inline void _start_worker(VIWorkerStatus* worker);
static inline VIWorkerStatus* _get_active_worker(const VIDispatcher* const restrict d);
static inline VIWorkerStatus* _get_worker(const VIDispatcher* const restrict d, const size_t i);
static inline VIWorkerStatus* _prepare_new_worker(
        VIDispatcher* const restrict d, const IrqLine func_index);

static VIError _init_worker(
        VIWorkerStatus* const restrict status,
        const wasm_module_inst_t module_inst,
        IrqFuncHandler* funcs,
        VIDispatcherSignalStatus* signal_status);

#define VI_ERROR_WAMR_NO_EXCEPTION  ""

int VI_ERROR_ERRNO;
const char* VI_ERROR_WAMR_EXCEPTION = VI_ERROR_WAMR_NO_EXCEPTION;

VIError vidispatcher_init(
        VIDispatcher* const restrict dispatcher,
        wasm_module_inst_t module_inst,
        wasm_function_inst_t main_f,
        const size_t n_lines,
        const size_t depth)
{
    VIError res = VIError_None;
    struct VIMainFunStatus*  main_f_status;
    SpanWorkerStatus workers = span_cfg_init(depth);
    IrqFuncHandler* funcs = NULL;
    size_t workers_ok=0;
    int err;

    if( !dispatcher || !module_inst || !depth || !main_f )
    {
        res = VIError_InvalidInput;
        goto end;
    }

//===================================--init memory==========================================
    funcs = malloc(n_lines * sizeof(*funcs));

    if ( !funcs )
    {
        res = VIError_Libc;
        VI_ERROR_ERRNO = errno;
        goto end;
    }

//======================================init queues==========================================
    spscq_init(&dispatcher->channel_ready_ureq);
    minheap_init(&dispatcher->minheap_ureq);

//======================================init signals=========================================
    struct sigaction sa ={0};
    sa.sa_sigaction = _th_irq_worker_signal_handler;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIG_PREEMPTION_WORKERS);

    if ( sigaction(SIG_PREEMPTION_WORKERS, &sa, NULL) < 0 )
    {
        res =VIError_Libc;
        VI_ERROR_ERRNO = errno;
        goto end;
    }

    if ( sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL) < 0 )
    {
        res =VIError_Libc;
        VI_ERROR_ERRNO = errno;
        goto end;
    }

//==================================init dispatcher conds/mutex=============================
    if ( (err = pthread_cond_init(&dispatcher->signal_status.cond, NULL)) )
    {
        res =VIError_Libc;
        errno = err;
        VI_ERROR_ERRNO = errno;
        goto end;
    }

    if ( (err = pthread_mutex_init(&dispatcher->signal_status.mutex, NULL)) )
    {
        res =VIError_Libc;
        errno = err;
        VI_ERROR_ERRNO = errno;
        goto end;
    }

//====================================init main function thread===============================

    main_f_status = &dispatcher->main_f_status;

    if ( (err = _init_preemption_status(&main_f_status->preemption_status) ) )
    {
        res =VIError_Libc;
        errno = err;
        VI_ERROR_ERRNO = errno;
        goto end;
    }

    main_f_status->p_dispatcher_signal_status = &dispatcher->signal_status;

    {
        int out = -1;
        ThMainThreadArg arg = {
            .module_inst = module_inst,
            .status = main_f_status,
            .out = &out,
            .main_f = main_f,
        };

        if( ( err = pthread_create(
                        &main_f_status->preemption_status.tid,
                        NULL,
                        _th_main_thread,
                        &arg) ) )
        {
            errno = err;
            VI_ERROR_ERRNO = errno;
            res = VIError_Libc;
            goto end;
        }

        //spinlock
        while(out == -1);

        if (out != VIError_None)
        {
            res = VIError_WAMR;
            goto end;
        }
    }

//======================================init workers==========================================
    {
        SpanError out_status; 
        span_resize(&workers, depth, &out_status);
        assert( out_status == SpanError_None && span_len(&workers) == depth );
        dispatcher->workers = workers;
    }
    for(size_t i=1; i<=depth; i++)
    {
        printf("VIDispatcher: init worker: %zu\n", i);
        VIWorkerStatus* worker = _get_worker(dispatcher, i);
        assert( worker );

        res = _init_worker(worker, module_inst, funcs, &dispatcher->signal_status);

        if ( res != VIError_None ) goto end;

        workers_ok++;
    }

//=========================================assigning field to dispatcher=======================
    dispatcher->n_funcs = n_lines;
    dispatcher->funcs = funcs;
    dispatcher->module_inst = module_inst;

    return res;

//=========================================error handling======================================
end:
    assert(res != VIError_None);
    for(size_t i=1; i<=workers_ok; i++)
    {
        VIWorkerStatus* worker = _get_worker(dispatcher, i);
        assert( worker );

        _destroy_preemption_status(&worker->preemption_status);

        pthread_mutex_destroy(&worker->data_mutex);
        pthread_cond_destroy(&worker->data_cond);
    }

    _destroy_signal_status(&dispatcher->signal_status);

    span_destroy(&dispatcher->workers);
    if (funcs) free(funcs);
    return res;
}

VIError vidispatcher_assign_irq_to_line(
        VIDispatcher* const restrict dispatcher,
        const IrqFuncHandler irq_handler,
        const size_t line)
{
    if( !dispatcher || line >= dispatcher->n_funcs)
    {
        return VIError_InvalidInput;
    }

    dispatcher->funcs[line] = irq_handler;

    return VIError_None;
}

VIError vidispatcher_start(VIDispatcher* const restrict dispatcher)
{
    int err;

    if ( !dispatcher )
    {
        return VIError_InvalidInput;
    }

    if( (err = pthread_create(&dispatcher->dispatcher_tid, NULL, _th_dispatcher , dispatcher)) < 0 )
    {
        errno = err;
        VI_ERROR_ERRNO = errno;
        return VIError_Libc;
    }

    return VIError_None;
}

VIError vidispatcher_trigger_interrupt(VIDispatcher* const restrict dispatcher, const IrqLine line)
{
    bool spsc_op_ok = false;

    if ( !dispatcher || line >= dispatcher->n_funcs ) return VIError_InvalidInput;

    spscq_push(&dispatcher->channel_ready_ureq, line, &spsc_op_ok);
    if ( !spsc_op_ok ) return VIError_Queue;

    _dispatcher_signal(&dispatcher->signal_status);

    return VIError_None;
}

void vidispatcher_destroy(VIDispatcher* const restrict dispatcher)
{
    if(dispatcher)
    {
        pthread_cancel(dispatcher->dispatcher_tid);
        pthread_join(dispatcher->dispatcher_tid, NULL);

        for(size_t i=1; i< span_len(&dispatcher->workers); i++)
        {
            VIWorkerStatus* worker = _get_worker(dispatcher, i);

            assert( worker );

            _destroy_preemption_status(&worker->preemption_status);

            pthread_mutex_destroy(&worker->data_mutex);
            pthread_cond_destroy(&worker->data_cond);
        }

        _destroy_signal_status(&dispatcher->signal_status);

        span_destroy(&dispatcher->workers);
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

static void* _th_main_thread(void* arg)
{
    uintptr_t res = VIError_None;
    ThMainThreadArg th_arg = *(ThMainThreadArg*) arg;

    wasm_module_inst_t module_inst = th_arg.module_inst;
    wasm_exec_env_t th_exec_env = {};
    sigset_t set = {};
    int err;

    //====================================init=================================================
    pthread_cleanup_push(_th_irq_worker_cleanup, &th_exec_env);
    wasm_runtime_init_thread_env();

    if( !module_inst )
    {
        res = VIError_InvalidInput;
        goto end;
    }

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
        VI_ERROR_ERRNO = errno;
        goto end;
    }
    *th_arg.out = VIError_None;

//=======================================logic=================================================
    th_arg.status->working = true;
    if ( wasm_runtime_call_wasm(th_exec_env, th_arg.main_f, 0, NULL) )
    {
        VI_ERROR_WAMR_EXCEPTION = wasm_runtime_get_exception(module_inst);
    }

    //INFO: if we reach here it means that the main has ended for any reason which is probably
    //an error unless the hole program ended
    _dispatcher_signal(th_arg.status->p_dispatcher_signal_status);

//=======================================end==================================================
end:
    pthread_cleanup_pop(true);
    return (void*) res;
}

static void* _th_irq_worker(void* arg)
{
    uintptr_t res = VIError_None;
    ThWorkersArg* th_arg = arg;

    VIWorkerStatus* status = th_arg->status;
    wasm_module_inst_t module_inst = th_arg->module_inst;
    wasm_exec_env_t th_exec_env = {};
    sigset_t set = {};
    int err;

//========================================init=================================================
    pthread_cleanup_push(_th_irq_worker_cleanup, &th_exec_env);
    wasm_runtime_init_thread_env();

    if( !module_inst )
    {
        res = VIError_InvalidInput;
        goto end;
    }

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
        VI_ERROR_ERRNO = errno;
        goto end;
    }

    th_arg->out = VIError_None;

//=======================================logic=================================================
    while(1)
    {
        VIDispatcherSignalStatus* disp_sig_ref = status->p_dispatcher_signal_status;
        
        assert(disp_sig_ref);

        pthread_mutex_lock(&status->data_mutex);
        {
            while( !atomic_load(&status->working) )
            {
                pthread_cond_wait(&status->data_cond, &status->data_mutex);
            }
        }
        pthread_mutex_unlock(&status->data_mutex);

        assert(status->p_funcs);

        printf("VIWorker: calling func: %zu\n", status->func_index);
        if ( !wasm_runtime_call_wasm(th_exec_env, status->p_funcs[status->func_index], 0, NULL) )
        {
            VI_ERROR_WAMR_EXCEPTION = wasm_runtime_get_exception(module_inst);
        }
        printf("VIWorker: finshed func: %zu\n", status->func_index);

        atomic_store(&status->working, false);

        _dispatcher_signal(disp_sig_ref);
    }

    //INFO: if we reach here it means that there is almost certain a problem
    //or the hole program ended

//=========================================end==================================================
end:
    th_arg->out = res;
    pthread_cleanup_pop(true);
    return (void*) res;
}

static void* _th_dispatcher(void* arg)
{
    uintptr_t res = VIError_None;
    VIDispatcher* dispatcher = arg;
    IrqLine ureq;
    VIWorkerStatus* old_worker, *new_worker;
    SPSCQ_UReq* c_ureq = &dispatcher->channel_ready_ureq;
    bool spscq_op_ok = false;

//========================================init==================================================
    assert(dispatcher);

//========================================logic=================================================
    while( true )
    {
        bool worker_finish  = false;

        //waiting for something to do
        if( spscq_is_empty(c_ureq) && !strcmp(VI_ERROR_WAMR_EXCEPTION,VI_ERROR_WAMR_NO_EXCEPTION) )
        {
            printf("VIDispatcher: waiting for something to do: "
                    "read: %ld, write: %ld, wamr_exception:--%s--\n",
                    atomic_load(&dispatcher->channel_ready_ureq.read),
                    atomic_load(&dispatcher->channel_ready_ureq.write),
                    VI_ERROR_WAMR_EXCEPTION
                  );
            pthread_mutex_lock(&dispatcher->signal_status.mutex);
            {
                pthread_cond_wait(&dispatcher->signal_status.cond, &dispatcher->signal_status.mutex);
            }
            pthread_mutex_unlock(&dispatcher->signal_status.mutex);
            printf("VIDispatcher: dispatcher weake up\n");
        }


        //old interrupt ended, unwinding execution to find suspended interrupt if it exists to
        //resume it
        while( dispatcher->executing_worker )
        {
            old_worker = _get_active_worker(dispatcher);
            if( old_worker && !atomic_load(&old_worker->working) )
            {
                dispatcher->executing_worker--;
                worker_finish  = true;
            }
            else if( worker_finish ) //resume stopped worker
            {
                printf("VIDispatcher: resuming suspended worker: %zu\n",
                        dispatcher->executing_worker);
                _preemption_status_signal(&old_worker->preemption_status);
                break;
            }
            else
            {
                break;
            }
        }

        //checking if user pushed an interrupt request
        //IF so AND the priority of the new req is > priority of the current executing_worker OR
        //there is no executing_worker THAN set suspended the current executing_worker and
        //set up a new executing_worker to handle the user request
        //IF the main was running it is be suspended
        spscq_pop(&dispatcher->channel_ready_ureq, &ureq, &spscq_op_ok);
        old_worker = _get_active_worker(dispatcher);
        if(spscq_op_ok && ( !old_worker || (size_t) ureq >= old_worker->func_index ) )
        {
            printf("VIDispatcher: user give new irq req: %zu\n", ureq);

            new_worker = _prepare_new_worker(dispatcher, ureq);
            assert(new_worker);

            //stop main board, if it's running
            _suspend_main_thread(&dispatcher->main_f_status);

            assert(!(dispatcher->executing_worker && dispatcher->main_f_status.working));

            //stop current worker (i)
            if( old_worker )
            {
                printf("VIDispatcher: suspending old worker: %zu\n",
                        dispatcher->executing_worker - 1);
                _preempt_thread(&old_worker->preemption_status);
            }

            //start new thread (i+1)
            printf("VIDispatcher: starting new worker: %zu\n", dispatcher->executing_worker);
            _start_worker(new_worker);
        }
        //ELSE IF the ureq < req that is already executing THAN save it on a wait queue for later
        else if ( spscq_op_ok )
        {
            bool minheap_push_ok = false;
            printf("VIDispatcher: user request %zu, has lower prority, saving on wait queue\n",
                    ureq);

            assert(ureq < old_worker->func_index);

            minheap_push(&dispatcher->minheap_ureq, &ureq, &minheap_push_ok);
            if ( !minheap_push_ok ) fprintf(stderr, "wait queue full, ureq lost");
        }

        //IF no worker is executing AND wait_queue is NOT empty THAN pop an ureq from wait queue
        //and assign it to a worker WHICH IS ALWAYS WORKER 1.
        if ( !dispatcher->executing_worker && !minheap_is_empty(&dispatcher->minheap_ureq))
        {
            bool pop_ok = false;

            printf("VIDispatcher: no worker active popping from wait queue: %zu\n", ureq);
            minheap_pop(&dispatcher->minheap_ureq, &ureq, &pop_ok);

            //pop MUST succeed since we just checked if the wait queue has elements in it 
            assert(pop_ok);

            new_worker = _prepare_new_worker(dispatcher, ureq);
            assert(new_worker);

            //stop main board, if it's running. It should not be needed but it cause no damage
            //to be sure
            _suspend_main_thread(&dispatcher->main_f_status);

            assert( dispatcher->executing_worker == 1 && !dispatcher->main_f_status.working );

            //start thread (1)
            printf("VIDispatcher: starting new worker from wait queue. (Req: %zu, Worker: %zu)\n",
                    ureq, dispatcher->executing_worker);
            _start_worker(new_worker);
        }

        assert(!(dispatcher->executing_worker && dispatcher->main_f_status.working));

        //IF no interrupt worker is running AND the main thread is not running
        //THAN resume the main thread
        if ( !dispatcher->executing_worker && !dispatcher->main_f_status.working )
        {
            printf("VIDispatcher: no worker is running, resuming main thread\n");
            _resume_thread_preemption(&dispatcher->main_f_status.preemption_status);
            dispatcher->main_f_status.working = true;
        }

        //IF and exception is caught signal it to the user and reset it
        if ( strcmp(VI_ERROR_WAMR_EXCEPTION,VI_ERROR_WAMR_NO_EXCEPTION) )
        {
            //TODO: logger
            fprintf(stderr, "thread error wamr call func: %s\n", VI_ERROR_WAMR_EXCEPTION);
            VI_ERROR_WAMR_EXCEPTION = VI_ERROR_WAMR_NO_EXCEPTION;
        }
    }

//====================================end==================================================
    return (void*) res;
}

static VIError _init_worker(
        VIWorkerStatus* const restrict status,
        const wasm_module_inst_t module_inst,
        IrqFuncHandler* funcs,
        VIDispatcherSignalStatus* signal_status)
{
    VIError res =VIError_None;

    int err;
    ThWorkersArg arg = {
        .module_inst = module_inst,
        .status = status,
        .out = -1};

    if ( (err = pthread_cond_init(&status->data_cond, NULL)) )
    {
        res =VIError_Libc;
        errno = err;
        VI_ERROR_ERRNO = errno;
        goto end;
    }

    if ( (err = pthread_mutex_init(&status->data_mutex, NULL)) )
    {
        res =VIError_Libc;
        errno = err;
        VI_ERROR_ERRNO = errno;
        goto end;
    }

    if ( (err = _init_preemption_status(&status->preemption_status)) )
    {
        res =VIError_Libc;
        errno = err;
        VI_ERROR_ERRNO = errno;
        goto end;
    }

    status->func_index = 0;
    status->p_dispatcher_signal_status = signal_status;
    status->p_funcs = funcs;
    atomic_init(&status->working, false);

    if( ( err = pthread_create(&status->preemption_status.tid, NULL, _th_irq_worker, &arg) ) )
    {
        errno = err;
        VI_ERROR_ERRNO = errno;
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

end:
    return res;
}

static inline void _suspend_main_thread(struct VIMainFunStatus* const restrict main_thread)
{
    if( main_thread->working )
    {
        printf("vidispatcher: suspending main thread\n");
        _preempt_thread(&main_thread->preemption_status);
        main_thread->working = false; 
    }
}

static inline int _init_preemption_status(VIPreemptionStatus* status)
{
    int res = 99;
    assert(status);

    if ( (res = pthread_mutex_init(&status->mutex, NULL)) ) return res;
    if ( (res = pthread_cond_init(&status->cond, NULL)) ) return res;

    return res;
}

static inline void _dispatcher_signal(VIDispatcherSignalStatus* const restrict status)
{
    assert(status);

    pthread_mutex_lock(&status->mutex);
    {
        pthread_cond_signal(&status->cond);
    }
    pthread_mutex_unlock(&status->mutex);
}

static inline void _preemption_status_signal(VIPreemptionStatus* status)
{
    pthread_mutex_lock(&status->mutex);
    {
        pthread_cond_signal(&status->cond);
    }
    pthread_mutex_unlock(&status->mutex);
}

static inline void _preemption_status_wait(VIPreemptionStatus* status)
{
    pthread_mutex_lock(&status->mutex);
    {
        pthread_cond_wait(&status->cond, &status->mutex);
    }
    pthread_mutex_unlock(&status->mutex);
}

static inline void _start_worker(VIWorkerStatus* worker)
{
    atomic_store(&worker->working, true);
    pthread_mutex_lock(&worker->data_mutex);
    {
        pthread_cond_signal(&worker->data_cond);
    }
    pthread_mutex_unlock(&worker->data_mutex);
}

static inline VIWorkerStatus* _prepare_new_worker(
        VIDispatcher* const restrict d, const IrqLine func_index)
{
    VIWorkerStatus* worker = NULL;
    SpanError span_out_status;

    assert(d);

    d->executing_worker++;

    if ( d->executing_worker >= span_len(&d->workers) )
    {
        const size_t new_stack_size = d->executing_worker << 1;
        VIError vi_error;

        assert( new_stack_size && new_stack_size > d->executing_worker );

        printf("VIDispatcher: reached stack limit, expanding to: %zu\n", new_stack_size);
        span_resize(&d->workers, new_stack_size, &span_out_status);
        assert( span_out_status == SpanError_None );

        for (size_t i=d->executing_worker + 1; i<=new_stack_size; i++)
        {
            worker = _get_worker(d, i);
            assert( worker );

            assert( d->module_inst );
            vi_error = _init_worker(worker, d->module_inst, d->funcs, &d->signal_status);
            assert(vi_error == VIError_None);
        }

    }


    worker = _get_active_worker(d);

    assert( worker );

    worker->func_index = func_index;

    return worker;
}

static inline VIWorkerStatus* _get_active_worker(const VIDispatcher* const restrict d)
{
    assert(d);
    return _get_worker(d, d->executing_worker);
}

static inline VIWorkerStatus* _get_worker(const VIDispatcher* const restrict d, const size_t i)
{
    VIWorkerStatus* res = NULL;
    SpanError span_res;

    assert(d);

    if ( i > 0 && i <= span_len(&d->workers) )
    {
        span_get(&d->workers, i - 1, &res, &span_res);

        assert( span_res == SpanError_None );
    }

    return res;
}

static inline void _preempt_thread(VIPreemptionStatus* const restrict status)
{
    assert(status);
    pthread_sigqueue(status->tid, SIG_PREEMPTION_WORKERS, (union sigval) {.sival_ptr = status});
    //TODO: check lifetime of pointer
}

static inline void _resume_thread_preemption(VIPreemptionStatus* const restrict status)
{
    assert(status);
    _preemption_status_signal(status);
}

static inline void _destroy_preemption_status(VIPreemptionStatus* const restrict status)
{
    assert(status);
    pthread_cancel(status->tid);
    pthread_join(status->tid, NULL);
    pthread_mutex_destroy(&status->mutex);
    pthread_cond_destroy(&status->cond);
}

static inline void _destroy_signal_status(VIDispatcherSignalStatus* const restrict status)
{
    assert(status);
    pthread_mutex_destroy(&status->mutex);
    pthread_cond_destroy(&status->cond);
}

static void _th_irq_worker_signal_handler(int signal, siginfo_t* info, void* ctx)
{
    assert(signal == SIG_PREEMPTION_WORKERS);
    (void) ctx;

    VIPreemptionStatus* status = info->si_value.sival_ptr;

    _preemption_status_wait(status);
}
