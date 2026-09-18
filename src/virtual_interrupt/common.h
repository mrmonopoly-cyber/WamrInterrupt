#pragma once

#include <assert.h>
#include <bits/types/sigset_t.h>
#include <signal.h>

#include "wasm_export.h"
#include "logger.h" //INFO: used by other modules

//============================================macros============================================

#ifndef VI_COMMON_PREFIX
#define VI_COMMON_PREFIX
#endif // !VI_COMMON_PREFIX

#define VI_ERROR_WAMR_NO_EXCEPTION  ""

#define VI_DEFAULT_SIG_SUSPEND  SIGPOLL
#define VI_DEFAULT_SIG_RESUME   SIGCONT

#define VI_RESULT_TYPE(T)                  T __attribute__((warn_unused_result))

#define VI_ASYNC_SIGNAL_HANDLER         __attribute__((annotate("async_signal_handler")))

#define VI_ASYNC_STATE                  __attribute__((capability("atomic_execution_state")))
#define VI_ASYNC_SETS_STATE(ret, ptr)   __attribute__((try_acquire_capability((ret), (ptr))))

#define VI_MAX_RETRIES (32ULL)
#define VI_LOOP_TRY(COUNTER_NAME, COND) \
    for( size_t (COUNTER_NAME) = 0; (COUNTER_NAME) < VI_MAX_RETRIES && (COND); (COUNTER_NAME)++ )

//============================================types============================================

typedef enum __VirtualInterruptError
{
    VIError_None=0,
    VIError_InvalidInput,
    VIError_Queue,
    VIError_Async,
    VIError_WAMR,
    VIError_Libc,           /* check errno */
}VIError;

typedef enum
{
    VISignals_Suspend,
    VISignals_Resume,

    VISignals_Count
}VISignals;

//============================================declarations======================================

VI_COMMON_PREFIX void _vi_set_errno(int error);
VI_COMMON_PREFIX void _vi_set_wamr_exception(wasm_module_inst_t module_inst);
VI_COMMON_PREFIX bool _vi_exists_wamr_exception(void);
VI_COMMON_PREFIX const char* _vi_get_wamr_exception(void);
VI_COMMON_PREFIX void _vi_clear_wamr_exception(void);
VI_COMMON_PREFIX VI_RESULT_TYPE(VIError) _vi_set_signal(VISignals signal, int val);
VI_COMMON_PREFIX VISignals _vi_get_signal(VISignals signal);
VI_COMMON_PREFIX const char* _vi_get_signal_name(VISignals signal);
VI_COMMON_PREFIX VI_RESULT_TYPE(VIError) _vi_enable_signal(VISignals signal);
VI_COMMON_PREFIX VI_RESULT_TYPE(VIError) _vi_disable_all_signals(void);
VI_COMMON_PREFIX const char* vi_error_to_str(const VIError err);


