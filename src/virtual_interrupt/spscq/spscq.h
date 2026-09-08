#pragma once

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#define SPSCQ_EXTRACT_CAP(QUEUE) ( sizeof((QUEUE)->data)/sizeof(*(QUEUE)->data) )

#define SPSCQ_ASSERT_CAP(QUEUE)                                                                 \
    static_assert(                                                                              \
            ( SPSCQ_EXTRACT_CAP((QUEUE)) & ( SPSCQ_EXTRACT_CAP(QUEUE) - 1 ) ) == 0,             \
            "CAPACITY must be a power of two")

#define SPSCQ_ASSERT_TYPES(T1, T2) \
    static_assert(__builtin_types_compatible_p(T1, T2), "invalid types")

#define TEMPLATE_SPSCQ(T, CAPACITY) \
    struct {T data[(CAPACITY)]; atomic_size_t read; atomic_size_t write;}\

#define spscq_init(QUEUE)                                                                       \
    do{                                                                                         \
        __typeof__ (*QUEUE)* p_queue = (QUEUE);                                                 \
        SPSCQ_ASSERT_CAP(p_queue);                                                              \
        atomic_init(&p_queue->read, 0);                                                         \
        atomic_init(&p_queue->write, 0);                                                        \
    }while(0);


//OUT_RES: true => push ok
//OUT_RES: false => push failed: queue is full
#define spscq_push(QUEUE, DATA, OUT_RES)                                                        \
    do{                                                                                         \
        __typeof__ (*QUEUE)* p_queue = (QUEUE);                                                 \
        bool* p_out_res = (OUT_RES);                                                            \
        SPSCQ_ASSERT_CAP(p_queue);                                                              \
        SPSCQ_ASSERT_TYPES(typeof(*p_queue->data), typeof((DATA)));                             \
        const size_t write = atomic_load_explicit(&p_queue->write, memory_order_relaxed);       \
        const size_t read = atomic_load_explicit(&p_queue->read, memory_order_acquire);         \
        const size_t next_write = (write + 1) & ( SPSCQ_EXTRACT_CAP(p_queue)  - 1 );            \
        if (p_out_res) *p_out_res = false;                                                      \
        if ( read != next_write )                                                               \
        {                                                                                       \
            p_queue->data[write] = (DATA);                                                      \
            atomic_store_explicit(&p_queue->write, next_write, memory_order_release);           \
            if (p_out_res) *p_out_res = true;                                                   \
        }                                                                                       \
    }while(0);


#define spscq_pop(QUEUE, OUT_PTR, OUT_RES)                                                      \
    do{                                                                                         \
        __typeof__ (*QUEUE)* p_queue = (QUEUE);                                                 \
        __typeof__ (OUT_PTR) p_out_ptr = (OUT_PTR);                                             \
        bool* p_out_res = (OUT_RES);                                                            \
        SPSCQ_ASSERT_CAP(p_queue);                                                              \
        SPSCQ_ASSERT_TYPES(typeof(*p_queue->data), typeof((*p_out_ptr)));                       \
        const size_t read = atomic_load_explicit(&p_queue->read, memory_order_relaxed);         \
        const size_t write = atomic_load_explicit(&p_queue->write, memory_order_acquire);       \
        const size_t next_read = (read + 1) & ( SPSCQ_EXTRACT_CAP(p_queue)  - 1 );              \
        if (p_out_res) *p_out_res = false;                                                      \
        if ( read != write )                                                                    \
        {                                                                                       \
            *p_out_ptr = p_queue->data[read];                                                   \
            atomic_store_explicit(&p_queue->read, next_read, memory_order_release);             \
            if (p_out_res) *p_out_res = true;                                                   \
        }                                                                                       \
    }while(0);

#define spscq_peek_last_pushed(QUEUE, OUT_PTR, OUT_RES)                                         \
    do{                                                                                         \
        __typeof__ (*QUEUE)* p_queue = (QUEUE);                                                 \
        __typeof__ (*OUT_PTR)* p_out_ptr = (OUT_PTR);                                           \
        bool* p_out_res = (OUT_RES);                                                            \
        SPSCQ_ASSERT_CAP(p_queue);                                                              \
        SPSCQ_ASSERT_TYPES(typeof(*p_queue->data), typeof((*p_out_ptr)));                       \
        const size_t cap = SPSCQ_EXTRACT_CAP((QUEUE));                                          \
        const size_t read = atomic_load_explicit(&p_queue->read, memory_order_acquire);         \
        const size_t write = atomic_load_explicit(&p_queue->write, memory_order_acquire);       \
        const size_t prev_write = (write - 1) & (cap - 1);                                      \
        *p_out_res = false;                                                                     \
        if ( read != write )                                                                    \
        {                                                                                       \
            *p_out_ptr = p_queue->data[prev_write];                                             \
            *p_out_res = true;                                                                  \
        }                                                                                       \
    }while(0);

#define spscq_is_empty(QUEUE)                                                                   \
    (                                                                                           \
        atomic_load_explicit(&(QUEUE)->write, memory_order_relaxed) ==                          \
        atomic_load_explicit(&(QUEUE)->read, memory_order_relaxed)                              \
    )


#ifdef SPSCQ_TEST
#include <stdio.h>

void spscq_test()
{
    typedef TEMPLATE_SPSCQ(uint8_t, 4) SPSCQ_Test;
    bool op_ok = false;
    uint8_t pred=0;

    SPSCQ_Test sd;
    const uint8_t data_push[SPSCQ_EXTRACT_CAP(&sd)] = {21, 42};
    uint8_t data_pop[SPSCQ_EXTRACT_CAP(&sd)] = {};

    spscq_init(&sd);

    spscq_peak_last_pushed(&sd, &pred, &op_ok);
    assert(pred == 0 && !op_ok);
    spscq_push(&sd, data_push[0], &op_ok);
    spscq_peak_last_pushed(&sd, &pred, &op_ok);
    assert(pred == data_push[0] && op_ok);
    assert(op_ok);

    spscq_push(&sd, data_push[1], &op_ok);
    spscq_peak_last_pushed(&sd, &pred, &op_ok);
    assert(pred == data_push[1] && op_ok);
    assert(op_ok);


    spscq_pop(&sd, &data_pop[0], &op_ok);
    spscq_peak_last_pushed(&sd, &pred, &op_ok);
    assert(pred == data_push[1] && op_ok);
    spscq_pop(&sd, &data_pop[1], &op_ok);
    spscq_peak_last_pushed(&sd, &pred, &op_ok);
    assert(pred == data_push[1] && !op_ok);
    
    printf("queue: cap: %lu, data[0]: %d, data[1]: %d\n",
            SPSCQ_EXTRACT_CAP(&sd), sd.data[0], sd.data[1]);
    printf("data push: %d, %d\n", data_push[0], data_push[1]);
    printf("data pop: %d, %d\n", data_pop[0], data_pop[1]);

    for(size_t i=0; i<SPSCQ_EXTRACT_CAP(&sd); i++)
    {
        assert(data_push[i] == data_pop[i] );
    }
}

int main()
{
    spscq_test();
    return 0;
}

#endif
