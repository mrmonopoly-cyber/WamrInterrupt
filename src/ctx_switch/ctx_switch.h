#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define STACK_SIZE (16ULL << 10)

typedef uintptr_t Reg;
typedef void (*UserFunc)(void* arg);

typedef struct
{
    void* base_addr;
    size_t size;
}Stack;

typedef struct
{
    Reg rsp;
}Context;

Stack create_new_stack(void);

Context context_self(void);
void context_init(Context* ctx, Stack* stack, UserFunc init_f, void* arg);

void __attribute__((__naked__)) context_switch(
    Context* const restrict old_cs __attribute__((__unused__)),
    const Context* const restrict new_cs __attribute__((__unused__)));

void destroy_stack(Stack* stack);



//========================================test=================================================
void test_ctx_switch(void);
