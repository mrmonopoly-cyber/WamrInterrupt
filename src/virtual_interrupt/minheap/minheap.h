#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#define MINHEAP_EXTRACT_CAP(MINHEAP) ( sizeof((MINHEAP)->data) / sizeof(*(MINHEAP)->data) )

#define MINHEAP_ASSERT_TYPES(T1, T2) \
    static_assert(__builtin_types_compatible_p(T1, T2), "invalid types")

#define MINHEAP_TEMPLATE(T, CAP) struct {T data[(CAP)]; size_t len; }

#define MINHEAP_STATIC_INIT {}

#define minheap_init(MINHEAP)                                                                   \
    do{                                                                                         \
        *(MINHEAP) = (__typeof__ (*MINHEAP)) {};                                                \
    }while(0);

#define minheap_push(MINHEAP, DATA, OUT_RES)                                                    \
    do{                                                                                         \
        __typeof__ (*MINHEAP)* _h = (MINHEAP);                                                  \
        bool* p_out_res = (OUT_RES);                                                            \
                                                                                                \
        MINHEAP_ASSERT_TYPES(__typeof__(*_h->data), __typeof__(*DATA));                         \
                                                                                                \
        *p_out_res = false;                                                                     \
        if ( _h->len < MINHEAP_EXTRACT_CAP(_h) )                                                \
        {                                                                                       \
            size_t new_data_pos = _h->len++;                                                    \
            __typeof__(*_h->data) new_data = *(DATA);                                           \
                                                                                                \
            *p_out_res = true;                                                                  \
            if ( new_data_pos == 0 )                                                            \
            {                                                                                   \
                _h->data[new_data_pos] = new_data;                                              \
                break;                                                                          \
            }                                                                                   \
                                                                                                \
            size_t parent_pos = (new_data_pos - 1) >> 1;                                        \
            __typeof__(*_h->data) parent_data = _h->data[parent_pos];                           \
                                                                                                \
            while(new_data < parent_data)                                                       \
            {                                                                                   \
                assert(parent_pos < MINHEAP_EXTRACT_CAP(_h));                                   \
                assert(new_data_pos < MINHEAP_EXTRACT_CAP(_h));                                 \
                                                                                                \
                _h->data[parent_pos] = new_data;                                                \
                _h->data[new_data_pos] = parent_data;                                           \
                                                                                                \
                new_data_pos = parent_pos;                                                      \
                parent_pos = new_data_pos ? (new_data_pos - 1) >> 1 : 0;                        \
                                                                                                \
                new_data = _h->data[new_data_pos];                                              \
                parent_data = _h->data[parent_pos];                                             \
            }                                                                                   \
        }\
    }while(0);

#define minheap_pop(MINHEAP, OUT_DATA, OUT_RES)                                                 \
    do{                                                                                         \
        __typeof__ (*MINHEAP)* _h = (MINHEAP);                                                  \
        __typeof__ (*OUT_DATA)* p_out_data= (OUT_DATA);                                         \
        bool* p_out_res = (OUT_RES);                                                            \
                                                                                                \
        MINHEAP_ASSERT_TYPES(__typeof__(*_h->data), __typeof__(*OUT_DATA));                     \
                                                                                                \
        *p_out_res = false;                                                                     \
                                                                                                \
        if ( _h->len == 0 ) break;                                                              \
                                                                                                \
        *p_out_res = true;                                                                      \
        *p_out_data = _h->data[0];                                                              \
        _h->data[0] = _h->data[--_h->len];                                                      \
                                                                                                \
        if ( _h->len == 0 ) break;                                                              \
                                                                                                \
        size_t _idx = 0;                                                                        \
        while(true)                                                                             \
        {                                                                                       \
            size_t _left_idx = (_idx << 1) + 1;                                                 \
            size_t _right_idx = (_idx << 1) + 2;                                                \
            size_t _smallest_idx = _idx;                                                        \
                                                                                                \
            if ( _left_idx < _h->len && _h->data[_left_idx] < _h->data[_smallest_idx])          \
            {                                                                                   \
                _smallest_idx = _left_idx;                                                      \
            }                                                                                   \
            if ( _right_idx < _h->len && _h->data[_right_idx] < _h->data[_smallest_idx] )       \
            {                                                                                   \
                _smallest_idx = _right_idx;                                                     \
            }                                                                                   \
            if ( _idx == _smallest_idx )                                                        \
            {                                                                                   \
                break;                                                                          \
            }                                                                                   \
            __typeof__(*(_h->data)) _tmp = _h->data[_idx];                                      \
            _h->data[_idx] = _h->data[_smallest_idx];                                           \
            _h->data[_smallest_idx] = _tmp;                                                     \
                                                                                                \
            _idx = _smallest_idx;                                                               \
        }                                                                                       \
    }while(0);



#ifdef ENABLE_TESTS
#include <stdio.h>

__attribute__((__constructor__))
void minheap_test()
{
    typedef MINHEAP_TEMPLATE(size_t, 12) MinheapChar;

    MinheapChar minheap;
    bool op_ok = false;
    size_t exp_temp =0;
    size_t temp =0;

    printf("running %s test\n", __func__);
    
    minheap_init(&minheap);

    minheap_pop(&minheap, &temp, &op_ok);
    printf("minheap pop: given: %zu, expected: %zu\n", temp, exp_temp);
    assert(temp == 0 && !op_ok);

    for(size_t i=0; i<MINHEAP_EXTRACT_CAP(&minheap); i++)
    {
        size_t to_insert = MINHEAP_EXTRACT_CAP(&minheap) - i;
        printf("minheap inserting %zu: %zu\n", i, to_insert);
        minheap_push(&minheap, &to_insert, &op_ok);
        assert(op_ok && minheap.data[0] == to_insert);
    }

    for(size_t i=0; i<MINHEAP_EXTRACT_CAP(&minheap); i++)
    {
        size_t to_find = i + 1;
        size_t out = 0;
        minheap_pop(&minheap, &out, &op_ok);
        printf("minheap pop %zu: given: %zu, expected: %zu\n", i, out, to_find);
        assert(op_ok && out == to_find);
    }

    minheap_pop(&minheap, &temp, &op_ok);
    printf("minheap pop: given: %zu, expected: %zu\n", temp, exp_temp);
    assert(temp == 0 && !op_ok);
}


#endif
