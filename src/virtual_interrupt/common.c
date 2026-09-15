#include "common.h"

int VI_ERROR_ERRNO;
const char* VI_ERROR_WAMR_EXCEPTION = VI_ERROR_WAMR_NO_EXCEPTION;

int VI_SIGNALS[VISignals_Count] = 
{
    [VISignals_Suspend] = VI_DEFAULT_SIG_SUSPEND,
    [VISignals_Resume] = VI_DEFAULT_SIG_RESUME,
};
