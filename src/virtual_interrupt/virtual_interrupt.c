#include "virtual_interrupt.h"

//=====================================includes===================================================
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"

#include "logger.h"
#include "minheap/minheap.h"
#include "workers/workers.h"
#include "irq_workers_list.h"

#define SPAN_IMPLEMENTATION
#include "span/span.h"
#include "spscq/spscq.h"
#include "wasm_export.h"

//=====================================macros====================================================

#define log(...) vi_log(VILoggerLevel_Info, log_buffer, sizeof(log_buffer), "Dispatcher", __VA_ARGS__)
#define log_warn(...) vi_log(VILoggerLevel_Warning, log_buffer, sizeof(log_buffer), "Dispatcher", __VA_ARGS__)
#define log_err(...) vi_log(VILoggerLevel_Error, log_buffer, sizeof(log_buffer),"Dispatcher",  __VA_ARGS__)

//=====================================types=====================================================

//=====================================function declarations======================================
static void* _th_dispatcher(void* arg);

static void _th_irq_worker_signal_handler(int signal);
static void _th_irq_resume_signal_handler(int signal);

static inline VIIrqWorkerStatus* _get_active_worker(const VIDispatcher* const restrict d);
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
    const size_t depth = conf.depth;
    char log_buffer[128] = {0};

    if (
            !dispatcher                                     ||
            !module_inst                                    ||
            !main_f                                         ||
            !depth                                          ||
            conf.suspend_signal == conf.resume_signal       ||
            !conf.log_file_base_path 
       )
    {
        res = VIError_InvalidInput;
        goto end;
    }

//========================================logger==================================================
    vi_log_file_init(conf.log_file_base_path);

//==========================================init memory===========================================
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
        const int sig_suspend = conf.suspend_signal;
        const int sig_resume = conf.resume_signal;

        if ( (res = _vi_set_signal(VISignals_Suspend, sig_suspend)) != VIError_None )
        {
            goto end;
        }

        if ( (res = _vi_set_signal(VISignals_Resume, sig_resume)) != VIError_None )
        {
            goto end;
        }

        if ( signal(sig_suspend, _th_irq_worker_signal_handler) ==  SIG_ERR )
        {
            res =VIError_Libc;
            _vi_set_errno(errno);
            goto end;
        }

        if ( signal(sig_resume, _th_irq_resume_signal_handler) ==  SIG_ERR )
        {
            res =VIError_Libc;
            _vi_set_errno(errno);
            goto end;
        }

        if ( _vi_disable_all_signals() != VIError_None )
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
        log("init worker: %zu", i);
        VIIrqWorkerStatus* worker = _get_worker(&dispatcher->workers, i);
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
    }

//=========================================assigning field to dispatcher=======================
    dispatcher->n_funcs = n_lines;
    dispatcher->funcs = funcs;
    dispatcher->module_inst = module_inst;

    return res;

//=========================================error handling======================================
end:
    assert(res != VIError_None);

    vidispatcher_destroy(dispatcher);
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
    const VIDispatcherConf default_conf = VIDISPATCHERCONF_DEFUALT;
    return vidispatcher_init_full(dispatcher, module_inst, main_f, n_lines, default_conf);
}

VIError vidispatcher_assign_irq_to_line(
        VIDispatcher* const restrict dispatcher,
        const IrqFuncHandler irq_handler,
        const size_t line)
{
    char log_buffer[64] = {0};

    if( !dispatcher || line >= dispatcher->n_funcs)
    {
        return VIError_InvalidInput;
    }

    log("setting irq line %zu, to %p", line, irq_handler);
    dispatcher->funcs[line] = irq_handler;

    return VIError_None;
}

VIError vidispatcher_start(VIDispatcher* const restrict dispatcher)
{
    char log_buffer[64] = {0};

    if ( !dispatcher )
    {
        return VIError_InvalidInput;
    }

    vi_worker_status_set_working_mode(&dispatcher->dispatcher.base, WorkerStatus_Init);
    log("starting dispatcher");
    vi_main_logic_resume(&dispatcher->main_fun_status);

    while( vi_worker_status_get_working_mode(&dispatcher->main_fun_status.base) == WorkerStatus_Init )
    {
        log("%s waiting main thread start", __func__);
        usleep(1000 * 1000);
    }

    int err =  vi_dispatcher_status_init(&dispatcher->dispatcher, _th_dispatcher, dispatcher);

    while( vi_worker_status_get_working_mode(&dispatcher->dispatcher.base) == WorkerStatus_Init )
    {
        log("%s waiting dispatcher thread to start", __func__);
        usleep(1000 * 1000);
    }

    if( err < 0 )
    {
        _vi_set_errno(err);
        return VIError_Libc;
    }

    return VIError_None;
}

VIError vidispatcher_trigger_interrupt(VIDispatcher* const restrict dispatcher, const IrqLine line)
{
    bool spsc_op_ok = false;
    char log_buffer[64] = {0};

    if ( !dispatcher || line >= dispatcher->n_funcs ) return VIError_InvalidInput;

    spscq_push(&dispatcher->channel_ready_ureq, line, &spsc_op_ok);
    if ( !spsc_op_ok ) return VIError_Queue;

    log("user triggered new interrupt on line: %zu", line);
    return vi_dispatcher_status_signal(&dispatcher->dispatcher);
}

void vidispatcher_destroy(VIDispatcher* const restrict dispatcher)
{
    char log_buffer[64]= {0};

    if( dispatcher )
    {
        log("destroying main thread");
        vi_main_logic_destroy(&dispatcher->main_fun_status);

        log("destroying dispatcher");
        vi_dispatcher_status_destroy(&dispatcher->dispatcher);

        FOR_EACH_IRQ_WORKER_INDEX(i, &dispatcher->workers)
        {
            VIIrqWorkerStatus* worker = _get_worker(&dispatcher->workers, i);
            WorkerStatus w_status = vi_irq_worker_get_mode(worker);

            if ( w_status != WorkerStatus_Init && w_status != WorkerStatus_Dead )
            {
                log("destroying workder: %zu", i);
                vi_irq_worker_destroy(worker);
            }
        }

        span_destroy(&dispatcher->workers);
        if ( dispatcher->funcs ) free(dispatcher->funcs);
    }
}

static void* _th_dispatcher(void* arg)
{
    uintptr_t res = VIError_None;
    VIDispatcher* dispatcher = arg;
    VIIrqWorkerStatus* old_worker, *new_worker;
    bool spscq_op_ok = false;
    char log_buffer[128] = {0};

//========================================init==================================================
    assert(dispatcher);
    vi_worker_status_set_working_mode(&dispatcher->dispatcher.base, WorkerStatus_Working);

    if ( _vi_disable_all_signals() != VIError_None )
    {
        log_err("failed to disable signals");
    }

//========================================logic=================================================
    while( true )
    {
start_dispatcher_loop:
        //waiting for something to do
        if ( atomic_load(&dispatcher->dispatcher.n_requests) == 0 )
        {
            log(
                    "waiting for something to do: read: %ld, write: %ld, wamr_exception:--%s--",
                    atomic_load(&dispatcher->channel_ready_ureq.read),
                    atomic_load(&dispatcher->channel_ready_ureq.write),
                    _vi_get_wamr_exception()
               );
            vi_worker_status_set_working_mode(&dispatcher->dispatcher.base, WorkerStatus_Suspended);

            if ( _vi_enable_signal(VISignals_Suspend) != VIError_None )
            {
                log_err("error enabling signal: %s\n", _vi_get_signal_name(VISignals_Suspend));
            }

            if ( _vi_enable_signal(VISignals_Resume) != VIError_None )
            {
                log_err("error enabling signal: %s\n", _vi_get_signal_name(VISignals_Resume));
            }

            vi_worker_status_self_suspend();

            if ( _vi_disable_all_signals() != VIError_None )
            {
                log_err("failed to disable signals");
            }

            vi_worker_status_set_working_mode(&dispatcher->dispatcher.base, WorkerStatus_Working);
            log("woke up");
        }

        if ( !atomic_load(&dispatcher->dispatcher.run) )
        {
            log("received termination request, stopping execution");
            break;
        }

        if ( atomic_load(&dispatcher->dispatcher.n_requests) == 0 )
        {
            log_err("INVARIANT NOT RESPECTED: n_requests == 0");
            continue;
        }

        atomic_fetch_sub(&dispatcher->dispatcher.n_requests, 1);

        if ( !atomic_load(&dispatcher->dispatcher.run) )
        {
            log("received termination request");
            break;
        }


        //old interrupt ended, unwinding execution to find suspended interrupt if it exists to
        //resume it
        bool worker_finish  = false;
        log("checking if worker finished: current depth: %zu",
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
                log("doing the stuck unwinding: current depth %zu",
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

                    log("starting new irq in waiting queue: %zu", ureq_pop);

                    minheap_pop(&dispatcher->minheap_ureq, &ureq_pop, &pop_ok);
                    assert( pop_ok && ureq_pop == ureq_peek );

                    new_worker = _prepare_new_worker(dispatcher, ureq_pop);

                    assert( new_worker );

                    vi_irq_worker_resume(new_worker);
                }
                else
                {
                    log("resuming suspended worker: %zu",
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
            log("user give new irq req: %zu", ureq);

            new_worker = _prepare_new_worker(dispatcher, ureq);
            assert(new_worker);

            //stop main board, if it's running
            vi_main_logic_suspend(&dispatcher->main_fun_status);

            //stop current worker (i)
            if( old_worker )
            {
                log("suspending old worker: %zu",
                        dispatcher->executing_worker - 1);
                vi_irq_worker_suspend(old_worker);
            }

            //start new thread (i+1)
            log("starting new worker: %zu", dispatcher->executing_worker);
            vi_irq_worker_resume(new_worker);
        }
        //ELSE IF the ureq < req that is already executing THAN save it on a wait queue for later
        else if ( spscq_op_ok )
        {
            bool minheap_push_ok = false;
            log("user request %zu, has lower prority, saving on wait queue", ureq);

            assert(ureq < old_worker->func_index);

            minheap_push(&dispatcher->minheap_ureq, &ureq, &minheap_push_ok);
            if ( !minheap_push_ok ) fprintf(stderr, "wait queue full, ureq lost");
        }

        //IF no worker is executing AND wait_queue is NOT empty THAN pop an ureq from wait queue
        //and assign it to a worker WHICH IS ALWAYS WORKER 1.
        if ( !dispatcher->executing_worker && !minheap_is_empty(&dispatcher->minheap_ureq))
        {
            bool pop_ok = false;

            log("no worker active popping from wait queue: %zu", ureq);
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
            log("starting new worker from wait queue. (Req: %zu, Worker: %zu)",
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
            log("no worker is running");
            vi_main_logic_resume(&dispatcher->main_fun_status);
        }

        //IF and exception is caught signal it to the user and reset it
        if ( _vi_exists_wamr_exception() )
        {
            //TODO: logger
            log_err("thread error wamr call func: %s\n", _vi_get_wamr_exception());
            _vi_clear_wamr_exception();
        }

    }

//====================================end==================================================
    vi_worker_status_set_working_mode(&dispatcher->dispatcher.base, WorkerStatus_Dead);
    log("dead");
    return (void*) res;
}


//=====================================helper functions==========================================
static inline VIIrqWorkerStatus* _prepare_new_worker(
        VIDispatcher* const restrict d, const IrqLine func_index)
{
    VIIrqWorkerStatus* worker = NULL;
    SpanError span_out_status;
    char log_buffer[128] = {0};

    assert(d);

    d->executing_worker++;

    if ( d->executing_worker >= span_len(&d->workers) )
    {
        const size_t new_stack_size = d->executing_worker << 1;
        VIError vi_error;

        assert( new_stack_size && new_stack_size > d->executing_worker );

        log("reached stack limit, expanding to: %zu", new_stack_size);
        span_resize(&d->workers, new_stack_size, &span_out_status);
        assert( span_out_status == SpanError_None );

        for (size_t i=d->executing_worker + 1; i<=new_stack_size; i++)
        {
            worker = _get_worker(&d->workers, i);
            assert( worker );

            assert( d->module_inst );
            log("init new worker: %zu", i);
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
    return _get_worker(&d->workers, d->executing_worker);
}

const char* vidispatcher_error_to_str(const VIError err)
{
    return vi_error_to_str(err);
}

//=====================================signal handlers============================================
static void _th_irq_worker_signal_handler(int signal)
{
    //NOLINTNEXTLINE(bugprone-signal-handler)
    if ( (VISignals) signal == _vi_get_signal(VISignals_Suspend) )
    {
        vi_worker_status_self_suspend(); //NOLINT(bugprone-signal-handler)
    }

}

static void _th_irq_resume_signal_handler(int signal)
{
    (void) signal;
    /*does nothing*/
}
