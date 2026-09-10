#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define SPAN_ASSERT_TYPES(T1, T2) \
    static_assert(__builtin_types_compatible_p(T1, T2), "invalid types")

#ifndef SPAN_CHUNK_SIZE
#define SPAN_CHUNK_SIZE 8
#endif // !SPAN_CHUNK_SIZE

#ifndef SPAN_TYPES
#define SPAN_TYPES
typedef enum
{
    SpanError_None=0,
    SpanError_Libc,
    SpanError_InvalidInput,
    SpanError_OutOfBounds
} SpanError;

struct __SpanCommon
{
    void** chunks;
    size_t chunk_size;
    size_t cap;
    size_t len;
};
#endif // !SPAN_TYPES

#define SPAN_TEMPLATE(T)                                                        \
    struct{                                                                     \
        struct __SpanCommon common;                                             \
        const T* _data_type;                                                    \
    }

#define span_cfg_init(CHUNK_SIZE) {{NULL, (CHUNK_SIZE), 0, 0}, NULL}

#define span_resize(SPAN, I, OUT_STATUS)                                        \
    do{                                                                         \
        __typeof__(*(SPAN))* _s = (SPAN);                                       \
        *(OUT_STATUS) = __span_resize(                                          \
                &_s->common,                                                    \
                (I),                                                            \
                sizeof(*_s->_data_type));                                       \
    }while(0)


#define span_write(SPAN, I, DATA, OUT_STATUS)                                   \
    do{                                                                         \
        __typeof__(*(SPAN))* _s = (SPAN);                                       \
        __typeof__(*(DATA))* _d = (DATA);                                       \
        SPAN_ASSERT_TYPES(__typeof__(*_s->_data_type), __typeof__(*_d));        \
        *(OUT_STATUS) = __span_write(                                           \
                &_s->common,                                                    \
                (I),                                                            \
                (const void*) (_d),                                             \
                sizeof(*_s->_data_type));                                       \
    }while(0)

#define span_get(SPAN, I, OUT_DATA, OUT_STATUS)                                 \
    do{                                                                         \
        __typeof__(*(SPAN))* _s = (SPAN);                                       \
        __typeof__(*(OUT_DATA))* _out = (OUT_DATA);                             \
        SPAN_ASSERT_TYPES(__typeof__(*_s->_data_type), __typeof__(**_out));     \
        *(OUT_STATUS) = __span_get(                                             \
                &_s->common,                                                    \
                (I),                                                            \
                (void**) (_out),                                                \
                sizeof(*_s->_data_type));                                       \
    }while(0)

#define span_len(SPAN) ( (SPAN)->common.len )


#define span_destroy(SPAN) __span_destroy(&(SPAN)->common)

SpanError __span_resize(
        struct __SpanCommon* span,
        const size_t i,
        const size_t ele_size);

SpanError __span_write(
        struct __SpanCommon* span,
        const size_t i,
        const void* data,
        const size_t ele_size);

SpanError __span_get(
        const struct __SpanCommon* const restrict span,
        const size_t i,
        void** out,
        const size_t ele_size);

void __span_destroy(const struct __SpanCommon* const restrict span);

//============================================implementation===================================

#ifdef SPAN_IMPLEMENTATION

SpanError __span_resize(
        struct __SpanCommon* span,
        const size_t len,
        const size_t ele_size)
{
    assert(ele_size);
    if(!span || !ele_size ) return SpanError_InvalidInput;

    if (!span->chunk_size) span->chunk_size = SPAN_CHUNK_SIZE;

    const size_t chunk_index = len / span->chunk_size;

    if (chunk_index > span->cap) 
    {
        const size_t new_cap = chunk_index + 1;

        void** new_chunks = (void**) realloc(span->chunks, new_cap * sizeof(*new_chunks));
        if (!new_chunks) return SpanError_Libc;

        for (size_t c = span->cap; c < new_cap; c++)
        {
            new_chunks[c] = malloc(span->chunk_size * ele_size);
            if ( !new_chunks[c] )
            {
                span->chunks = new_chunks;
                span->cap = c;
                return SpanError_Libc; 
            }
        }

        span->chunks = new_chunks;
        span->cap = new_cap;
    }
    
    span->len = len;

    return SpanError_None;
}

SpanError __span_write(
        struct __SpanCommon* span, const size_t i, const void* data, const size_t ele_size)
{
    SpanError res = SpanError_None;

    assert(ele_size);
    if(!span || !data) return SpanError_InvalidInput;

    if (!span->chunk_size) span->chunk_size = SPAN_CHUNK_SIZE;

    const size_t chunk_index = i / span->chunk_size;
    const size_t chunk_offset = i % span->chunk_size;
    
    if ( (res = __span_resize(span, i, ele_size)) ) return res;

    uint8_t* target_chunk = (uint8_t*)span->chunks[chunk_index];
    memcpy(target_chunk + (chunk_offset * ele_size), data, ele_size);

    return SpanError_None;
}

SpanError __span_get(
        const struct __SpanCommon* const restrict span,
        const size_t i, void** out, const size_t ele_size)
{
    assert(ele_size);
    if(!span || !out ) return SpanError_InvalidInput;
    if(i >= span->len || !span->chunk_size ) return SpanError_OutOfBounds;

    const size_t chunk_index = i / span->chunk_size;
    const size_t chunk_offset = i % span->chunk_size;

    uint8_t* target_chunk = (uint8_t*)span->chunks[chunk_index];
    *out = target_chunk + (chunk_offset * ele_size);

    return SpanError_None;
}

void __span_destroy(const struct __SpanCommon* const restrict span)
{
    if( !span || !span->chunks ) return;

    for (size_t i=0; i< span->cap; i++)
    {
        printf("span free chunk: %zu\n", i);
        if( span->chunks[i] ) free(span->chunks[i]);
    }

    printf("span free chunk root\n");
    free(span->chunks);
}

#endif // SPAN_IMPLEMENTATION

//===========================================tests=============================================

#if defined(ENABLE_TESTS) && !defined(SPAN_TESTS)
#define SPAN_TESTS
#include <stdio.h>
#include <stdint.h>

__attribute__((__constructor__))
void test_span()
{
    typedef SPAN_TEMPLATE(uintptr_t) SpanUintPtr;

    printf("running %s test\n", __func__);

    SpanUintPtr span = span_cfg_init(11);
    SpanError err = SpanError_None;
    uintptr_t* data;

    for(uintptr_t i = 0; i < 30; i++)
    {
        span_write(&span, i, &i, &err);
        assert(err == SpanError_None);
    }

    for(size_t i = 0; i < 30; i++)
    {
        span_get(&span, i, &data, &err);
        printf("get span_status: %d, expected: %zu, got: %zu\n", err, i, *data);
        assert(err == SpanError_None && *data == (uintptr_t) i);
    }

    span_destroy(&span);
}
#endif
