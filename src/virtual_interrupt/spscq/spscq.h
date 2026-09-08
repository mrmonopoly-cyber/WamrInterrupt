#pragma once

#include <assert.h>
#include <stdatomic.h>
#include <stdint.h>

#define EXTRACT_CAP(QUEUE) ( sizeof((QUEUE)->data)/sizeof(*(QUEUE)->data) )

#define ASSERT_CAP(QUEUE)                                                                       \
    static_assert(                                                                              \
            ( EXTRACT_CAP((QUEUE)) & ( EXTRACT_CAP(QUEUE) - 1 ) ) == 0,                         \
            "CAPACITY must be a power of two")

#define ASSERT_TYPES(T1, T2) static_assert(__builtin_types_compatible_p(T1, T2), "invalid types")

#define TEMPLATE_SPSCQ(T, CAPACITY) \
    struct {T data[(CAPACITY)]; atomic_size_t read; atomic_size_t write;}\

#define spscq_init(QUEUE)                                                                       \
    do{                                                                                         \
        __typeof__ (*QUEUE)* p_queue = (QUEUE);                                                 \
        ASSERT_CAP(p_queue);                                                                    \
        atomic_init(&p_queue->read, 0);                                                         \
        atomic_init(&p_queue->write, 0);                                                        \
    }while(0);


#define spscq_push(QUEUE, DATA)                                                                 \
    do{                                                                                         \
        __typeof__ (*QUEUE)* p_queue = (QUEUE);                                                 \
        ASSERT_CAP(p_queue);                                                                    \
        ASSERT_TYPES(typeof(*p_queue->data), typeof((DATA)));                                   \
        const size_t write = atomic_load_explicit(&p_queue->write, memory_order_acquire);       \
        const size_t read = atomic_load_explicit(&p_queue->read, memory_order_relaxed);         \
        const size_t next_write = (write + 1) & ( EXTRACT_CAP(p_queue)  - 1 );                  \
        if ( read != next_write )                                                               \
        {                                                                                       \
            p_queue->data[write] = (DATA);                                                      \
            atomic_store_explicit(&p_queue->write, next_write, memory_order_release);           \
        }                                                                                       \
    }while(0);

#define spscq_is_empty(QUEUE)                                                                   \
    (                                                                                           \
        atomic_load_explicit(&c_ureq->write, memory_order_relaxed) ==                           \
        atomic_load_explicit(&c_ureq->read, memory_order_relaxed)                               \
    )


#define spscq_pop(QUEUE, OUT_PTR)                                                               \
    do{                                                                                         \
        __typeof__ (*QUEUE)* p_queue = (QUEUE);                                                 \
        ASSERT_CAP(p_queue);                                                                    \
        ASSERT_TYPES(typeof(*p_queue->data), typeof((*OUT_PTR)));                               \
        const size_t read = atomic_load_explicit(&p_queue->read, memory_order_relaxed);         \
        const size_t write = atomic_load_explicit(&p_queue->write, memory_order_acquire);       \
        const size_t next_read = (read + 1) & ( EXTRACT_CAP(p_queue)  - 1 );                    \
        if ( read != write )                                                                    \
        {                                                                                       \
            *(OUT_PTR) = p_queue->data[read];                                                   \
            atomic_store_explicit(&p_queue->read, next_read, memory_order_release);             \
        }                                                                                       \
    }while(0);



#ifdef SPSCQ_TEST
#include <stdio.h>

void spscq_test()
{
    typedef TEMPLATE_SPSCQ(uint8_t, 4) SPSCQ_Test;

    SPSCQ_Test sd;
    const uint8_t data_push[EXTRACT_CAP(&sd)] = {21, 42};
    uint8_t data_pop[EXTRACT_CAP(&sd)] = {};

    spscq_init(&sd);

    spscq_push(&sd, data_push[0]);
    spscq_push(&sd, data_push[1]);

    spscq_pop(&sd, &data_pop[0]);
    spscq_pop(&sd, &data_pop[1]);
    
    printf("queue: cap: %lu, data[0]: %d, data[1]: %d\n",
            EXTRACT_CAP(&sd), sd.data[0], sd.data[1]);
    printf("data push: %d, %d\n", data_push[0], data_push[1]);
    printf("data pop: %d, %d\n", data_pop[0], data_pop[1]);

    for(size_t i=0; i<EXTRACT_CAP(&sd); i++)
    {
        assert(data_push[i] == data_pop[i]);
    }
}

int main()
{
    spscq_test();
    return 0;
}

#endif
