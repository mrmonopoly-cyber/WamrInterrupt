#pragma once

#include <stdint.h>
#include <stdatomic.h>

#ifndef LB_BRIDGE_PREFIX
#define LB_BRIDGE_PREFIX
#endif // !LB_BRIDGE_PREFIX

typedef enum
{
    Executor_MainThread = 0,
    Executor_Interrupt
}Executor;

LB_BRIDGE_PREFIX void board_set_executor(const int32_t executor);
LB_BRIDGE_PREFIX Executor launcher_get_executor(void);

//===========================================implementation=====================================
#ifdef LB_BRIDGE_IMPLEMENTATION

atomic_int EXECUTOR;

__attribute__((__constructor__))
void __EXECUTOR_INIT(void)
{
    atomic_init(&EXECUTOR, 0);
}

LB_BRIDGE_PREFIX void board_set_executor(const int32_t executor)
{
    extern atomic_int EXECUTOR;

    atomic_store(&EXECUTOR, executor);
}

LB_BRIDGE_PREFIX Executor launcher_get_executor(void)
{
    extern atomic_int EXECUTOR;

    return atomic_load(&EXECUTOR);
}

#endif // LB_BRIDGE_IMPLEMENTATION
