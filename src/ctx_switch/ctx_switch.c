#include "ctx_switch.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void __panic(void)
{
    fprintf(stderr, "user space thread: panic!\n");
    exit(0);
}

void __attribute__((__naked__)) __trampoline(void)
{
    __asm__ volatile(
        "mov %rbp, %rdi\n\r"     // user_arg
        "call *%rbx\n\r"         // call init_func(user_arg)
        "call __panic\n\r"
    );
}

static void context_init(
        Context* const restrict ctx,
        Stack* stack,
        UserFunc init_func,
        void* user_arg)
{
  uintptr_t sp_addr = 0;
  void** sp = NULL;

  sp_addr = (uintptr_t) ((char*)stack->base_addr + stack->size);
  sp_addr = sp_addr & ~0xF; //rsp % 16 == 8

  sp = (void**) sp_addr;

  *(--sp) = (void*) (uintptr_t) __panic;
  *(--sp) = (void*) (uintptr_t) __trampoline;
  *(--sp) = init_func;    //rbx
  *(--sp) = user_arg;     //rbp
  *(--sp) = 0;            //r11
  *(--sp) = 0;            //r12
  *(--sp) = 0;            //r13
  *(--sp) = 0;            //r14
  *(--sp) = 0;            //r15

  ctx->rsp = (Reg) sp;
}


void __attribute__((__naked__)) context_switch(
    Context* const restrict old_cs __attribute__((__unused__)),
    const Context* const restrict new_cs __attribute__((__unused__)))
{
    __asm__ volatile(
      "push %rbx\n\r"
      "push %rbp\n\r"
      "push %r11\n\r"
      "push %r12\n\r"
      "push %r13\n\r"
      "push %r14\n\r"
      "push %r15\n\r"

      "mov %rsp, (%rdi)\n\r"
      "mov (%rsi), %rsp\n\r"

      "pop %r15\n\r" 
      "pop %r14\n\r"
      "pop %r13\n\r"
      "pop %r12\n\r"
      "pop %r11\n\r"
      "pop %rbp\n\r"
      "pop %rbx\n\r"

      "ret"
      );
}


int create_new_stack(Context* ctx, Stack* stack, UserFunc init_f, void* user_arg)
{
    void* base_addr;

    assert(ctx);

    if( !(base_addr = malloc(STACK_SIZE)) ) goto fail;

    *stack = (Stack){
        .base_addr = base_addr,
        .size = STACK_SIZE,
    };

    context_init(ctx, stack, init_f, user_arg);

    return 0;

fail:
    return 1;
}

void destroy_stack(Stack* stack)
{
    assert(stack);
    free(stack->base_addr);
}

//========================================test=================================================
typedef struct
{
    Context* parent;
    Context* child;
}Gemini;

static void coroutine(void* arg)
{
    Gemini* gemini = (Gemini*) arg;

    assert(gemini);

    while(1)
    {
        printf("coroutine works\n");
        context_switch(gemini->child, gemini->parent);
    }
}

void test_ctx_switch(void)
{
    Context parent_ctx = {0};
    Context child_ctx = {0};
    Stack child_stack = {0};

    Gemini gemini = 
    {
        .parent = &parent_ctx,
        .child = &child_ctx,
    };

    create_new_stack(&child_ctx, &child_stack, coroutine, &gemini);

    context_switch(&parent_ctx, &child_ctx);
    context_switch(&parent_ctx, &child_ctx);
    context_switch(&parent_ctx, &child_ctx);


    destroy_stack(&child_stack);

}
