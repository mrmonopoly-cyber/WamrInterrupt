#include "virtual_interrupt.h"

//=====================================includes===================================================
#include <assert.h>
#include <bits/types/sigset_t.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "minheap/minheap.h"
#include "workers/base.h"
#include "workers/dispatcher.h"
#include "workers/irq_worker.h"
#include "workers/main_logic.h"
#define SPAN_IMPLEMENTATION
#include "span/span.h"
#include "spscq/spscq.h"
#include "wasm_export.h"

//=====================================macros====================================================

#define FOR_EACH_IRQ_WORKER_INDEX(NAME, WORKERS) \
    for (size_t NAME = 1; i <= (span_len(WORKERS)); i++)

#define FOR_EACH_IRQ_WORKER_RANGE(NAME, MAX) \
    for (size_t NAME = 1; i <= (MAX); i++)

//=====================================types=====================================================

//=====================================function declarations======================================
static void* _th_dispatcher(void* arg);

static void _th_irq_worker_signal_handler(int signal);
static void _th_irq_resume_signal_handler(int signal);

static inline VIIrqWorkerStatus* _get_active_worker(const VIDispatcher* const restrict d);
static inline VIIrqWorkerStatus* _get_worker(const VIDispatcher* const restrict d, const size_t i);
static inline VIIrqWorkerStatus* _prepare_new_worker(
        VIDispatcher* const restrict d, const IrqLine func_index);

//=========================================implementation=========================================

VIError vidispatcher_init_full(
        VIDispatcher* const restrict dispatcher,
        wasm_module_inst_t module_inst,
        wasm_function_inst_t main_f,
        const size_t n_lines,
        const VIDispatcherConf conf)
{
    VIError res = VIError_None;
    IrqFuncHandler* funcs = NULL;
    size_t workers_ok=0;
    sigset_t set;
    const size_t depth = conf.depth;

    if (
            !dispatcher             ||
            !module_inst            ||
            !main_f                 ||
            !depth
       )
    {
        res = VIError_InvalidInput;
        goto end;
    }

    sigemptyset(&set);

    if ( (res = _vi_set_signal(VISignals_Suspend, conf.suspend_signal)) != VIError_None )
    {
        goto end;
    }

    if ( (res = _vi_set_signal(VISignals_Resume, conf.resume_signal)) != VIError_None )
    {
        goto end;
    }

//=====================================init memory===========================================
    funcs = malloc(n_lines * sizeof(*funcs));

    if ( !funcs )
    {
        _vi_set_errno(errno);
        goto end;
    }

//======================================init queues==========================================
    spscq_init(&dispatcher->channel_ready_ureq);
    minheap_init(&dispatcher->minheap_ureq);

//======================================init signals=========================================
    {
        const int sig_suspend = _vi_get_signal(VISignals_Suspend);
        const int sig_resume = _vi_get_signal(VISignals_Resume);
        struct sigaction sa ={0};
        sigemptyset(&sa.sa_mask);

        sa.sa_handler = _th_irq_worker_signal_handler;
        if ( sigaction(sig_suspend, &sa, NULL) < 0 )
        {
            res =VIError_Libc;
            _vi_set_errno(errno);
            goto end;
        }

        sa.sa_handler = _th_irq_resume_signal_handler;
        if ( sigaction(sig_resume, &sa, NULL) < 0 )
        {
            res =VIError_Libc;
            _vi_set_errno(errno);
            goto end;
        }

        sigaddset(&sa.sa_mask, sig_resume);
        sigaddset(&sa.sa_mask, sig_suspend);
        if ( pthread_sigmask(SIG_BLOCK, &sa.sa_mask, NULL) < 0 )
        {
            res =VIError_Libc;
            _vi_set_errno(errno);
            goto end;
        }
    }

//====================================init main function thread===============================

    res = vi_main_logic_init(&dispatcher->main_fun_status, &dispatcher->dispatcher, main_f, module_inst);
    if ( res != VIError_None) 
    {
        goto end;
    }

//======================================init workers==========================================
    {
        SpanError out_status; 
        VISpanWorkerStatus workers = span_cfg_init(depth);
        span_resize(&workers, depth, &out_status);
        if ( out_status != SpanError_None || span_len(&workers) != depth ) 
        {
            res = VIError_Queue;
            goto end;
        }
        dispatcher->workers = workers;
    }

    for(size_t i=1; i<=depth; i++)
    {
        printf("VIDispatcher: init worker: %zu\n", i);
        VIIrqWorkerStatus* worker = _get_worker(dispatcher, i);
        assert( worker );

        if ((
                    res = vi_irq_worker_init(
                        worker,
                        &dispatcher->dispatcher,
                        funcs,
                        module_inst)
            ) != VIError_None )
        {
            goto end;
        }

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
    FOR_EACH_IRQ_WORKER_RANGE(i, workers_ok)
    {
        VIIrqWorkerStatus* worker = _get_worker(dispatcher, i);
        assert( worker );
        vi_irq_worker_destroy(worker);
    }

    vi_main_logic_destroy(&dispatcher->main_fun_status);

    span_destroy(&dispatcher->workers);
    if (funcs) free(funcs);
    return res;
}

VIError vidispatcher_init(
        VIDispatcher* const restrict dispatcher,
        wasm_module_inst_t module_inst,
        wasm_function_inst_t main_f,
        const size_t n_lines
        )
{
    const VIDispatcherConf default_conf =
    {
        .depth = 8,
        .suspend_signal = VI_DEFAULT_SIG_SUSPEND,
        .resume_signal = VI_DEFAULT_SIG_RESUME,
    };
    return vidispatcher_init_full(dispatcher, module_inst, main_f, n_lines, default_conf);
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

    if ( !dispatcher )
    {
        return VIError_InvalidInput;
    }

    int err =  vi_dispatcher_status_init(&dispatcher->dispatcher, _th_dispatcher, dispatcher);

    if( err < 0 )
    {
        _vi_set_errno(err);
        return VIError_Libc;
    }

    vi_main_logic_resume(&dispatcher->main_fun_status);

    return VIError_None;
}

VIError vidispatcher_trigger_interrupt(VIDispatcher* const restrict dispatcher, const IrqLine line)
{
    bool spsc_op_ok = false;

    if ( !dispatcher || line >= dispatcher->n_funcs ) return VIError_InvalidInput;

    spscq_push(&dispatcher->channel_ready_ureq, line, &spsc_op_ok);
    if ( !spsc_op_ok ) return VIError_Queue;

    vi_dispatcher_status_signal(&dispatcher->dispatcher);

    return VIError_None;
}

void vidispatcher_destroy(VIDispatcher* const restrict dispatcher)
{
    if(dispatcher)
    {
        vi_dispatcher_status_destroy(&dispatcher->dispatcher);

        FOR_EACH_IRQ_WORKER_INDEX(i, &dispatcher->workers)
        {
            VIIrqWorkerStatus* worker = _get_worker(dispatcher, i);
            vi_irq_worker_destroy(worker);
        }

        vi_main_logic_destroy(&dispatcher->main_fun_status);

        span_destroy(&dispatcher->workers);
        if ( dispatcher->funcs ) free(dispatcher->funcs);

        *dispatcher = (VIDispatcher) {0};
    }
}

const char* vi_error_to_str(const VIError err)
{
    extern int VI_ERROR_ERRNO;
    switch (err)
    {
        case VIError_None:                  return "";
        case VIError_InvalidInput:          return "invalid input";
        case VIError_Queue:                 return "Internal Queue error: Full?";
        case VIError_WAMR:                  return "wamr error";
        case VIError_Libc:                  return "libc error";
    }

    assert(0 && "unreachable");
}

static void* _th_dispatcher(void* arg)
{
    uintptr_t res = VIError_None;
    VIDispatcher* dispatcher = arg;
    VIIrqWorkerStatus* old_worker, *new_worker;
    bool spscq_op_ok = false;

//========================================init==================================================
    assert(dispatcher);

//========================================logic=================================================
    while( true )
    {
start_dispatcher_loop:
        //waiting for something to do
        if ( atomic_load(&dispatcher->dispatcher.n_requests) == 0 )
        {
            printf("VIDispatcher: waiting for something to do: "
                    "read: %ld, write: %ld, wamr_exception:--%s--\n",
                    atomic_load(&dispatcher->channel_ready_ureq.read),
                    atomic_load(&dispatcher->channel_ready_ureq.write),
                    _vi_get_wamr_exception()
                  );
            vi_worker_status_self_suspend();
            printf("VIDispatcher: dispatcher woke up\n");
        }

        atomic_fetch_sub(&dispatcher->dispatcher.n_requests, 1);


        //old interrupt ended, unwinding execution to find suspended interrupt if it exists to
        //resume it
        bool worker_finish  = false;
        printf("VIDispatcher: checking if worker finished: current depth: %zu\n",
                dispatcher->executing_worker);
        while( dispatcher->executing_worker )
        {
            old_worker = _get_active_worker(dispatcher);
            if(
                    old_worker &&
                    vi_irq_worker_get_mode(old_worker) == WorkerStatus_Done
              )
            {
                dispatcher->executing_worker--;
                worker_finish  = true;
                printf("VIDispatcher: doing the stuck unwinding: current depth %zu\n",
                        dispatcher->executing_worker);
            }
            else if( worker_finish ) //resume stopped worker
            {
                bool minheap_has_something = false;
                IrqLine ureq_peek = 0;
                IrqLine ureq_pop = 0;

                minheap_peek(&dispatcher->minheap_ureq, &ureq_peek, &minheap_has_something);

                old_worker = _get_active_worker(dispatcher);
                if ( minheap_has_something && ureq_peek > atomic_load(&old_worker->func_index) )
                {
                    VIIrqWorkerStatus* new_worker;
                    bool pop_ok = false;

                    printf("VIDispatcher: starting new irq in waiting queue: %zu\n", ureq_pop);

                    minheap_pop(&dispatcher->minheap_ureq, &ureq_pop, &pop_ok);
                    assert( pop_ok && ureq_pop == ureq_peek );

                    new_worker = _prepare_new_worker(dispatcher, ureq_pop);

                    assert( new_worker );

                    vi_irq_worker_resume(new_worker);
                }
                else
                {
                    printf("VIDispatcher: resuming suspended worker: %zu\n",
                            dispatcher->executing_worker);
                    vi_irq_worker_resume(old_worker);
                }

                goto start_dispatcher_loop;
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
        IrqLine ureq;
        spscq_pop(&dispatcher->channel_ready_ureq, &ureq, &spscq_op_ok);
        old_worker = _get_active_worker(dispatcher);
        if(spscq_op_ok && ( !old_worker || (size_t) ureq >= old_worker->func_index ) )
        {
            printf("VIDispatcher: user give new irq req: %zu\n", ureq);

            new_worker = _prepare_new_worker(dispatcher, ureq);
            assert(new_worker);

            //stop main board, if it's running
            vi_main_logic_suspend(&dispatcher->main_fun_status);

            //stop current worker (i)
            if( old_worker )
            {
                printf("VIDispatcher: suspending old worker: %zu\n",
                        dispatcher->executing_worker - 1);
                vi_irq_worker_suspend(old_worker);
            }

            //start new thread (i+1)
            printf("VIDispatcher: starting new worker: %zu\n", dispatcher->executing_worker);
            vi_irq_worker_resume(new_worker);
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
            vi_main_logic_suspend(&dispatcher->main_fun_status);

            assert(
                    dispatcher->executing_worker == 1 &&
                    vi_main_logic_get_mode(&dispatcher->main_fun_status) == WorkerStatus_Suspended
                  );

            //start thread (1)
            printf("VIDispatcher: starting new worker from wait queue. (Req: %zu, Worker: %zu)\n",
                    ureq, dispatcher->executing_worker);
            vi_irq_worker_resume(new_worker);
        }

        assert(!(
                    dispatcher->executing_worker > 0 &&
                    vi_main_logic_get_mode(&dispatcher->main_fun_status) == WorkerStatus_Working
                ));

        //IF no interrupt worker is running AND the main thread is not running
        //THAN resume the main thread
        if (
                !dispatcher->executing_worker &&
                vi_main_logic_get_mode(&dispatcher->main_fun_status) == WorkerStatus_Suspended
           )
        {
            printf("VIDispatcher: no worker is running\n");
            vi_main_logic_resume(&dispatcher->main_fun_status);
        }

        //IF and exception is caught signal it to the user and reset it
        if ( _vi_exists_wamr_exception() )
        {
            //TODO: logger
            fprintf(stderr, "thread error wamr call func: %s\n", _vi_get_wamr_exception());
            _vi_clear_wamr_exception();
        }

    }

//====================================end==================================================
    return (void*) res;
}


//=====================================helper functions==========================================
static inline VIIrqWorkerStatus* _prepare_new_worker(
        VIDispatcher* const restrict d, const IrqLine func_index)
{
    VIIrqWorkerStatus* worker = NULL;
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
            vi_error = vi_irq_worker_init(worker, &d->dispatcher, d->funcs, d->module_inst);
            assert(vi_error == VIError_None);
        }

    }


    worker = _get_active_worker(d);

    assert( worker );

    atomic_store(&worker->func_index, func_index);

    return worker;
}

static inline VIIrqWorkerStatus* _get_active_worker(const VIDispatcher* const restrict d)
{
    assert(d);
    return _get_worker(d, d->executing_worker);
}

static inline VIIrqWorkerStatus* _get_worker(const VIDispatcher* const restrict d, const size_t i)
{
    VIIrqWorkerStatus* res = NULL;
    SpanError span_res;

    assert(d);

    if ( i > 0 && i <= span_len(&d->workers) )
    {
        span_get(&d->workers, i - 1, &res, &span_res);

        assert( span_res == SpanError_None );
    }

    return res;
}

//=====================================signal handlers============================================
static void _th_irq_worker_signal_handler(int signal)
{
    assert(signal == (int) _vi_get_signal(VISignals_Suspend));

    write(STDOUT_FILENO, "thread %zu, suspending:\n", pthread_self());
    vi_worker_status_self_suspend();
    write(STDOUT_FILENO, "thread %zu, resuming:\n", pthread_self());
}

static void _th_irq_resume_signal_handler(int signal)
{
    assert(signal == (int) _vi_get_signal(VISignals_Resume));

    /*does nothing*/
}
