/*  dynarray.h - typesafe dynamic array library 
    version 0.3.6 - Iain Dorsch - 2026

    To use this library, do the following in *one* of your .c files:
        #define DYNARRAY_IMPLEMENTATION
        #include "dynarray.h"
    In any other .c file that needs the library, just include the header:
        #include "dynarray.h"

    Too avoid collisions with other libraries, the standard API uses a "da_" prefix.
    Short names are also provided. 
    If prefix names are required short names can be disabled:
        #define DYNARRAY_NO_SHORT_NAMES

    Unit tests can be enabled by including the library as described above and 
    defining DYNARRAY_UNIT_TESTS. The function _da_arr_unit_tests() must then be called to run the tests.
    Example:
        #define DYNARRAY_IMPLEMENTATION
        #define DYNARRAY_UNIT_TESTS
        #include "dynarray.h"

        int main() {
            _da_arr_unit_tests();
            return 0;
        }

    Custom allocation can be used by defining the custom allocator flag DA_ARR_CUSTOM_ALLOC and defining both realloc and free.
    This also changes the behavior of the init functions of the array, 
    which now will require a pointer to the allocator used as a parameter.
    Example:
        #define DYNARRAY_IMPLEMENTATION
        #define DA_ARR_CUSTOM_ALLOC
        #define DA_REALLOC(allocator,pointer,size) arena_realloc(allocator,pointer,size)
        #define DA_FREE(allocator,pointer) arena_free(pointer)
        #include "dynarray.h"

        int main() {
            // example allocator
            arena_t * arena_buffer = arena_init(40000);
            // changed array init requiring passing of allocator pointer
            dynarray_t * list_one = da_arr(int,arena_buffer);
            return 0;
        }
    
    Functionality provided by this library:
        - Dynamic array of any type
        - Amortized O(1) push and pop operations
        - O(1) get and set operations
        - O(1) swap remove
        - O(n) insert and remove
        - O(n + m) concatenation
        - O(n) partitioning
        - O(n) mapping and filtering
        - O(n) rotate like c++ rotate
        - O(n) find first item maching condition (first_cond)
            can be made to be upper or lower bound by passing the according comparison function.
        - insertion sort using rotate and upper_bound.
            upper_bound is created by passing a a_greater_than_b function as the condition to first_cond.
            thus a_greater_than_b must be provided to insertion sort.
        - Unit tests
        - realloc and free can be custom defined
        - print all elements with specified format.
            example for array of int: 
                da_arr_print_all(int, list_one, " %d;",*item);

    Non allocation functionality is implemented on a Slice meaning all that funcitonality
    can be used with slices and functions to create slices from dynarrays is provided.
    Slices also enable the use of these functionalities for other list types by providing
    a pointer to the start of the list and the length of the list.

    Table of contents:
        - Library instructionss
        - short name API
        - Internal functions declarations
        - prefix name API
        - Internal functions implementations
        - Unit tests
*/

#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <stddef.h>

#define DYNARRAY_VERSION "0.3.0"
#define DA_UNUSED(...) (void)(__VA_ARGS__)
#define DA_MIN(a,b) ((a) < (b) ? (a) : (b))
#define DA_MAX(a,b) ((a) > (b) ? (a) : (b))

#if defined(DA_REALLOC) && !defined(DA_FREE) || !defined(DA_REALLOC) && defined(DA_FREE)
#error "The custom allocation flag (DA_ARR_CUSTOM_ALLOC) must be defined and both realloc and free must be defined, one is not sufficient."
#endif
#if !defined(DA_REALLOC) && !defined(DA_FREE)
#include <stdlib.h>
#define DA_REALLOC(allocator,pointer,size) realloc(pointer,size)
#define DA_FREE(allocator,pointer) free(pointer)
#endif

#ifndef DA_ARR_MIN_CAPACITY
#define DA_ARR_MIN_CAPACITY 1000
#endif

typedef struct {
    size_t end;
    char * start;
} slice_t;

typedef struct {
    size_t start;
    size_t end;
} range_t;

typedef struct {
    size_t capacity;
    size_t cnt;
    #ifdef DA_ARR_CUSTOM_ALLOC
    void * allocator;
    #endif
    char * data;
} dynarray_t;

// // // // // // // // // // // // // // //
extern void _da_arr_unit_tests(void);
// // // // // // // // // // // // // // // 
// Short names API
#ifndef DYNARRAY_NO_SHORT_NAMES
// arr macros init
#define arr_reserve                 da_arr_reserve
#define arr                         da_arr
#define arr_with                    da_arr_with
// arr macros functionality
#define arr_set_value               da_arr_set_value
#define arr_set                     da_arr_set
#define arr_foreach                 da_arr_foreach
#define arr_filter_each             da_arr_filter_each
#define arr_map                     da_arr_map
#define arr_len                     da_arr_len
#define arr_capacity                da_arr_capacity
#define arr_is_empty                da_arr_is_empty
#define arr_clear                   da_arr_clear
#define arr_push                    da_arr_push
#define arr_push_value              da_arr_push_value
#define arr_free                    da_arr_free
#define arr_swap_remove             da_arr_swap_remove
#define arr_filter_remove_unstable  da_arr_filter_remove_unstable
#define arr_pop                     da_arr_pop
#define arr_get                     da_arr_get
#define arr_peek                    da_arr_peek
// #define arr_insert_value            da_arr_insert_value
#define arr_insert                  da_arr_insert
#define arr_remove                  da_arr_remove
#define arr_push_front              da_arr_push_front
#define arr_concat                  da_arr_concat
#define arr_swap_index              da_arr_swap_index
#define arr_partition               da_arr_partition
#define arr_partition_ex            da_arr_partition_ex
#define arr_ptr                     da_arr_ptr
#define arr_match_first             da_arr_match_first
#define arr_insertion_sort          da_arr_insertion_sort
#define arr_quick_sort              da_arr_quick_sort
#define arr_stack_quick_sort        da_arr_stack_quick_sort
// slice macros
#define slice_init                  da_slice_init
#define slice_from_arr              da_slice_from_arr
#define slice_from_arr_range        da_slice_from_arr_range
#define slice_set                   da_slice_set
#define slice_set_value             da_slice_set_value
#define slice_get                   da_slice_get
#define slice_foreach               da_slice_foreach
#define slice_map                   da_slice_map
#define slice_filter_each           da_slice_filter_each
#define slice_swap                  da_slice_swap
#define slice_rotate_swap           da_slice_rotate_swap
#define slice_match_first           da_slice_match_first
#define slice_insertion_sort        da_slice_insertion_sort
#define slice_partition             da_slice_partition
#define slice_partition_ex          da_slice_partition_ex
#define slice_quick_sort            da_slice_quick_sort
#define slice_stack_quick_sort      da_slice_stack_quick_sort

#endif
// // // // // // // // // // // // // // // 
// Internal functions
extern dynarray_t *_array_empty(void * allocator);
extern dynarray_t *_array_init(size_t size, size_t init_capacity, void * allocator);
extern dynarray_t * _array_init_with(size_t size, size_t count, void * elem, void * allocator);
extern void _array_push(size_t size, dynarray_t *arr, void *elem, size_t init_capacity, void * allocator);
extern void _array_swap_remove(size_t size, dynarray_t *arr, size_t idx, size_t init_capacity, void * allocator);
extern char *_array_pop(size_t size, dynarray_t *arr, size_t init_capacity, void * allocator);
extern void _array_free(dynarray_t *arr, void * allocator);
extern void _array_insert(size_t size, dynarray_t *arr, size_t idx, void *elem, size_t init_capacity, void * allocator);
extern void _array_remove(size_t size, dynarray_t *arr, size_t idx, size_t init_capacity, void * allocator);
extern void _array_concat(size_t size, dynarray_t *dest, dynarray_t *other, void * allocator);
// slice functions
extern void _slice_set(size_t size, slice_t *slice, size_t idx, void *elem);
extern char *_slice_get(size_t size, slice_t *slice, size_t idx);
extern void _slice_swap_index(size_t size, slice_t *slice, size_t idx_one, size_t idx_two);
extern void _slice_rotate_swap(size_t size, slice_t *slice, size_t first, size_t mid, size_t last);
// extern ptrdiff_t _slice_partition_range(size_t size, slice_t *slice, size_t start, size_t end, int (*f)(const char *, const char *));
extern void _slice_quick_sort(size_t size, slice_t *slice, size_t start, size_t end, int (*f)(const char *, const char *));
extern size_t da_log_two(size_t);

// // // // // // // // // // // // // // // 
// No short names API
// // 
// Allocation as parameter
// // init operations
#if defined(DA_ARR_CUSTOM_ALLOC)
// // // allocator specified
#define da_arr_reserve(Type, Allocator, size) (dynarray_t *)_array_init(sizeof(Type), size, Allocator)
#define da_arr(Type, Allocator) (dynarray_t *)_array_init(sizeof(Type), DA_ARR_MIN_CAPACITY, Allocator)
#define da_arr_with(Type, Allocator, count, elem) (dynarray_t *)_array_init_with(sizeof(Type), count, elem, Allocator)

#else
// // // no allocator specified
#define da_arr_reserve(Type, size) (dynarray_t *)_array_init(sizeof(Type), size,NULL)
#define da_arr(Type) (dynarray_t *)_array_init(sizeof(Type), DA_ARR_MIN_CAPACITY,NULL)
#define da_arr_with(Type, count, elem) (dynarray_t *)_array_init_with(sizeof(Type), count, elem,NULL)

#endif
// // slice macros
#define da_slice_init(start, end) _slice_init((char *)(start), end)
#define da_slice_from_arr(array) _slice_from_array(array)
#define da_slice_from_arr_range(Type, array, start, end) _slice_from_array_range(sizeof(Type),array,start,end)
#define da_arr_to_slice(array) ((slice_t){.end = (array)->cnt, .start = (array)->data})
#define da_slice_set_value(Type,slice,index,elem) _slice_set(sizeof(Type),slice,index,(void *)&(Type){(elem)})
#define da_slice_set(Type,slice,index,elem) _slice_set(sizeof(Type),slice,index,elem)
#define da_slice_get(Type,slice,index) (Type *)_slice_get(sizeof(Type),slice,index)
#define da_slice_swap_index(Type, slice, idx_a, idx_b) _slice_swap_index(sizeof(Type), slice, idx_a, idx_b) 
#define da_slice_foreach(Type, item, slice) for (Type *item = (Type *)(slice)->start; item < (Type *)((slice)->start + (slice)->end * sizeof(Type)); item++)
#define da_slice_map(Type, slice, item, operation) do{\
    for (Type *item = (Type *)(slice)->start; \
         item < (Type *)((slice)->start + (slice)->end * sizeof(Type)); \
         item++) {\
            {operation};\
         }\
}while(0)
#define da_slice_filter_each(Type, slice, item, condition, operation) do{\
    da_slice_map(Type,slice,item,{\
        if(condition){\
            {operation};\
        }\
    });\
}while(0)
#define inner_slice_match_first(Type, slice, first, last, item, condition, RESULT) do{\
    RESULT = -1;\
    for(size_t __i = first; __i<(size_t)last; __i++) {\
        Type* item = da_slice_get(Type,slice,__i);\
        if(condition) {\
            RESULT = __i;\
            break;\
        }\
    }\
}while(0)
#define da_slice_match_first(Type, slice, item, condition, RESULT) inner_slice_match_first(Type, slice, 0, (slice)->end, a, b, condition)
#define inner_slice_insertion_sort(Type, slice, first, last, a, b, condition) do{\
    Type * b;\
    ptrdiff_t new_first;\
    for (size_t i = 0; i < (size_t)last; i++) {\
        b = da_slice_get(Type, slice, i);\
        inner_slice_match_first(Type, slice, first, i, a, condition, new_first);\
        if (new_first < 0) continue;\
        _slice_rotate_swap(sizeof(Type), slice, new_first, i, i+1);\
    }\
}while(0)
#define da_slice_insertion_sort(Type, slice, a, b, condition) inner_slice_insertion_sort(Type, slice, 0, (slice)->end, a, b, condition)
#define inner_slice_partition_range(Type, slice, start, end, pivot_idx, a, b, condition, RESULT) do{\
    Type *a;\
    Type *b;\
    RESULT = -1;\
    if (end > 0) {\
        size_t inner_end = end - 1;\
        if (inner_end <= start) return -1;\
        if (pivot_idx != inner_end) _slice_swap_index(sizeof(Type),slice,pivot_idx,inner_end);\
        Type * elem_pivot = (Type *)_slice_get(sizeof(Type),slice,inner_end);\
        ptrdiff_t i = start;\
        for (size_t j = start; j < inner_end; j++) {\
            Type * elem_j = (Type *)_slice_get(sizeof(Type),slice,j);\
            a=elem_j;\
            b=elem_pivot;\
            if (condition) {\
                _slice_swap_index(sizeof(Type),slice,i,j);\
                i++;\
            }\
        }\
        _slice_swap_index(sizeof(Type),slice,i,inner_end);\
        RESULT = i;\
    }\
}while(0)
#define da_slice_partition(Type,slice,a,b,condition,RESULT) inner_slice_partition_range(Type, slice, 0, (slice)->end, (slice)->end - 1, a, b, condition, RESULT)
#define da_slice_partition_ex(Type,slice,start,end,pivot_idx,a,b,condition,RESULT) inner_slice_partition_range(Type, slice, start, end, pivot_idx, a, b, condition, RESULT)
// range_t stack[da_log_two((slice)->end)];
#define inner_slice_stack_non_recursive_quick_sort(Type, slice, first, last, a, b, condition) do{\
    range_t stack[(slice)->end];\
    ptrdiff_t stack_idx = 0;\
    int not_done = 1;\
    range_t current_range = (range_t){.start = first, .end = last};\
    while(not_done) {\
        if (current_range.start < current_range.end-1) {\
            ptrdiff_t pi;\
            inner_slice_partition_range(Type, slice, current_range.start, current_range.end, current_range.end - 1, a, b, condition, pi);\
            if (pi > (ptrdiff_t)current_range.start+1) {\
                stack[stack_idx] = (range_t){.start = current_range.start, .end = pi};\
                stack_idx += 1;\
            }\
            if (pi < (ptrdiff_t)current_range.end-1) {\
                stack[stack_idx] = (range_t){.start = pi+1, .end = current_range.end};\
                stack_idx += 1;\
            }\
        }\
        if (stack_idx > 0) {\
            stack_idx -= 1;\
            current_range = stack[stack_idx];\
        } else {\
            not_done = 0;\
        }\
    }\
}while(0)
// range_t *stack = (range_t*)malloc(sizeof(range_t)*da_log_two((slice)->end));
#define inner_slice_alloc_non_recursive_quick_sort(Type, slice, first, last, a, b, condition) do{\
    range_t *stack = (range_t*)malloc(sizeof(range_t)*(slice)->end);\
    ptrdiff_t stack_idx = 0;\
    int not_done = 1;\
    range_t current_range = (range_t){.start = first, .end = last};\
    while(not_done) {\
        if (current_range.start < current_range.end-1) {\
            ptrdiff_t pi;\
            inner_slice_partition_range(Type, slice, current_range.start, current_range.end, current_range.end - 1, a, b, condition, pi);\
            if (pi > (ptrdiff_t)current_range.start+1) {\
                stack[stack_idx] = (range_t){.start = current_range.start, .end = pi};\
                stack_idx += 1;\
            }\
            if (pi < (ptrdiff_t)current_range.end-1) {\
                stack[stack_idx] = (range_t){.start = pi+1, .end = current_range.end};\
                stack_idx += 1;\
            }\
        }\
        if (stack_idx > 0) {\
            stack_idx -= 1;\
            current_range = stack[stack_idx];\
        } else {\
            not_done = 0;\
        }\
    }\
    free(stack);\
}while(0)
#define da_slice_quick_sort(Type, slice, a, b, condition) inner_slice_alloc_non_recursive_quick_sort(Type, slice, 0, (slice)->end, a, b, condition)
#define da_slice_stack_quick_sort(Type, slice, a, b, condition) inner_slice_stack_non_recursive_quick_sort(Type, slice, 0, (slice)->end, a, b, condition)
#define da_slice_print_all(Type,slice,item,...) do{\
    da_slice_map(Type,slice,item,{printf(__VA_ARGS__);});\
    printf("\n");\
}while(0)
// // // // // // // // // // // // // // // 
// // slice to array translation wrappers
#define inner_arr_use_slice(array,slice_operation) do{slice_t *slice = &da_arr_to_slice(array); slice_operation;}while(0)
#define da_arr_set_value(Type,array,index,elem) _slice_set(sizeof(Type),&da_arr_to_slice(array),index,(void *)&(Type){(elem)})
#define da_arr_set(Type,array,index,elem) _slice_set(sizeof(Type),&da_arr_to_slice(array),index,elem)
#define da_arr_get(Type,array,index) (Type *)_slice_get(sizeof(Type),&da_arr_to_slice(array),index)
#define da_arr_swap_index(Type, array, idx_a, idx_b) _slice_swap_index(sizeof(Type), &da_arr_to_slice(array), idx_a, idx_b) 
// #define da_arr_partition(Type,array,condition) _slice_partition_range(sizeof(Type),&da_arr_to_slice(array),0,array->cnt,(int(*)(const char *,const char *))(condition))
#define da_arr_partition(Type,array,a,b,condition,RESULT) inner_arr_use_slice(array,inner_slice_partition_range(Type,slice,0,(array)->cnt,(array)->cnt-1,a,b,condition,RESULT))
#define da_arr_partition_ex(Type,array,start,end,pivot_idx,a,b,condition,RESULT) inner_arr_use_slice(array,inner_slice_partition_range(Type,slice,start,end,pivot_idx,a,b,condition,RESULT))
#define da_arr_rotate_swap(Type,array,first,mid,last) _slice_rotate_swap(sizeof(Type), &da_arr_to_slice(array), first, mid, last)
#define da_arr_foreach(Type, item, array) da_slice_foreach(Type, item, &da_arr_to_slice(array))
#define da_arr_map(Type, array, item, operation) inner_arr_use_slice(array,da_slice_map(Type, slice, item, operation))
#define da_arr_filter_each(Type, array, item, condition, operation) inner_arr_use_slice(array,da_slice_filter_each(Type, slice, item, condition, operation))
#define da_arr_match_first(Type, array, first, last, item, condition, RESULT) inner_arr_use_slice(array,inner_slice_match_first(Type, slice, first, last, item, condition, RESULT))
#define da_arr_insertion_sort(Type, array, first, last, a, b, condition) inner_arr_use_slice(array,inner_slice_insertion_sort(Type, slice, first, last, a, b, condition))
#define da_arr_quick_sort(Type, array, first, last, a, b, condition) inner_arr_use_slice(array,inner_slice_alloc_non_recursive_quick_sort(Type, slice, first, last, a, b, condition))
#define da_arr_stack_quick_sort(Type, array, first, last, a, b, condition) inner_arr_use_slice(array,inner_slice_stack_non_recursive_quick_sort(Type, slice, first, last, a, b, condition))
#define da_arr_print_all(Type, array, item, ...) inner_arr_use_slice(array,da_slice_print_all(Type, slice, item, __VA_ARGS__))
// // // // // // // // // // // // // // // 
// // array utilities
#define da_arr_len(array) (array)->cnt
#define da_arr_capacity(array) (array)->capacity
#define da_arr_is_empty(array) ((array)->cnt == 0)
#define da_arr_clear(array) (array)->cnt = 0
#define da_arr_peek(Type,array) da_arr_get(Type,array,(array)->cnt-1)
#define da_arr_ptr(Type,array) ((Type*)((array)->data))
// // // // // // // // // // // // // // // 
// // internal operation allocation
#ifdef DA_ARR_CUSTOM_ALLOC
#define DA_ALLOCATOR(array) (array)->allocator
#else
#define DA_ALLOCATOR(array) NULL
#endif
// #define da_arr_push_value(Type,array,elem) _array_push(sizeof(Type),array,(void *)&(Type){(elem)}, DA_ARR_MIN_CAPACITY,DA_ALLOCATOR(array))
#define da_arr_push(Type,array,elem) _array_push(sizeof(Type),array,elem, DA_ARR_MIN_CAPACITY,DA_ALLOCATOR(array))
#define da_arr_push_value(Type,array,elem) do{Type item = (elem);_array_push(sizeof(Type),array,&(item), DA_ARR_MIN_CAPACITY,DA_ALLOCATOR(array));}while(0)
#define da_arr_push_front(Type,array,elem) da_arr_insert(Type,array,0,elem,DA_ALLOCATOR(array))
// #define da_arr_insert_value(Type,array,index,elem) _array_insert(sizeof(Type),array,index,(void *)&(Type){(elem)}, DA_ARR_MIN_CAPACITY,DA_ALLOCATOR(array))
#define da_arr_insert(Type,array,index,elem) _array_insert(sizeof(Type),array,index,elem, DA_ARR_MIN_CAPACITY,DA_ALLOCATOR(array))
#define da_arr_swap_remove(Type,array,index) _array_swap_remove(sizeof(Type),array,index, DA_ARR_MIN_CAPACITY,DA_ALLOCATOR(array))
#define da_arr_remove(Type,array,index) _array_remove(sizeof(Type),array,index, DA_ARR_MIN_CAPACITY,DA_ALLOCATOR(array))
#define da_arr_pop(Type,array) (Type *)_array_pop(sizeof(Type), array, DA_ARR_MIN_CAPACITY,DA_ALLOCATOR(array))
#define da_arr_concat(Type,array_one,array_two) _array_concat(sizeof(Type),array_one,array_two,DA_ALLOCATOR(array))
#define da_arr_free(array) _array_free(array,DA_ALLOCATOR(array))

#define da_arr_filter_remove_unstable(Type, array, item, condition) do{\
    size_t traverse = 0;\
    while(traverse < (array)->cnt) {\
        Type *item = (Type *)((array)->data + traverse * sizeof(Type));\
        if(condition) {\
            da_arr_swap_remove(Type, array, traverse);\
        }else{\
            traverse++;\
        }\
    }\
}while(0)
// // // // // // // // // // // // // // // 
// Internal functions implementations
#ifdef DYNARRAY_IMPLEMENTATION

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

size_t da_log_two(size_t x) {
    size_t log_two = 0;
    while (x >>= 1) ++log_two;
    return log_two;
}
// // // // // // // // // // // // // // // 
// slice functions and array wrappers for them
// slice functionality :
// set; get; swap_item; rotate_swap; partition_range;
slice_t _slice_init(char * start, size_t end) {
    return (slice_t){.end = end, .start = start};
}

slice_t _slice_from_array(dynarray_t * array) {
    return _slice_init(array->data,array->cnt);
}

slice_t _slice_from_array_range(size_t size, dynarray_t * array, size_t start, size_t end) {
    if (start + end <= array->cnt) {
        return _slice_init(array->data + size * start, end);
    }
    return _slice_init(array->data + size * start, array->cnt - start);
}

__attribute__((nonnull(2)))
void _slice_set(size_t size, slice_t *slice, size_t idx, void *elem) {
    if (idx < slice->end) {
        char *ptr = slice->start;
        ptr = ptr + (idx * size);
        memcpy(ptr, elem, size);
    }
}

__attribute__((hot,warn_unused_result,nonnull(2)))
char *_slice_get(size_t size, slice_t *slice, size_t idx) {
    char *ptr = NULL;
    if (idx < slice->end) {
        ptr = slice->start + (idx * size);
    }
    return ptr;
}

__attribute__((hot,nonnull(2)))
void _slice_swap_index(size_t size, slice_t *slice, size_t idx_one, size_t idx_two) {
    if (idx_one < slice->end && idx_two < slice->end && idx_one != idx_two) {
        char *a = _slice_get(size, slice, idx_one);
        char *b = _slice_get(size, slice, idx_two);
        unsigned char tmp[size];
        memcpy(tmp, a, size);
        memcpy(a, b, size);
        memcpy(b, tmp, size);
    }
}

void _slice_rotate_swap(size_t size, slice_t *slice, size_t first, size_t mid, size_t last) {
    if (first == mid) {
        return;
    }
    if (first >= last) {
        return;
    }

    size_t write = first; // 
    size_t read = mid; //
    size_t next_read = read; 
    // One unnecessary if execution at beginning of loop executing three lines below should skip first execution of if.
    // However, since the value is already the one which would be assigned in the first if could be irrelevant due to compiler optimization
    while(read < last) {
        if (write == next_read) {
            next_read = read;
        }
        _slice_swap_index(size, slice, write, read);
        write++;
        read++;
    }
    _slice_rotate_swap(size, slice, write, next_read, last);
}
// // // // // // // // // // // // // // // 
// array functions: requires capacity and/or allocation
__attribute__((warn_unused_result))
dynarray_t *_array_empty(void * allocator) {
    dynarray_t *result = (dynarray_t *)DA_REALLOC(allocator,NULL,sizeof(dynarray_t));
    
    memset(result, 0, sizeof(dynarray_t));
    return result;
}

__attribute__((warn_unused_result))
dynarray_t *_array_init(size_t size, size_t init_capacity, void * allocator) {
    dynarray_t *ptr = _array_empty(allocator);
    ptr->data = (char *)DA_REALLOC(allocator,NULL,size*init_capacity);
    // memset(ptr->data, 0, size*init_capacity);
    #ifdef DA_ARR_CUSTOM_ALLOC
    ptr->allocator = allocator;
    #endif
    ptr->capacity = init_capacity;
    return ptr;
}

__attribute__((nonnull(3),warn_unused_result))
dynarray_t * _array_init_with(size_t size, size_t count, void * elem, void * allocator) {
    dynarray_t *ptr = _array_empty(allocator);
    ptr->data = (char *)DA_REALLOC(allocator,NULL,size*count);
    #ifdef DA_ARR_CUSTOM_ALLOC
    ptr->allocator = allocator;
    #endif
    ptr->capacity = count;
    ptr->cnt = count;
    for (size_t i = 0; i < count; i++) {
        memcpy(ptr->data + size * i, elem, size);
    }
    return ptr;
}

__attribute__((nonnull(2)))
static void _array_resize(size_t size, dynarray_t *arr, int direction, size_t init_capacity, void * allocator) {
    switch (direction) {
        case 1:
            arr->data = (char *)DA_REALLOC(allocator, arr->data, size*arr->capacity*2);
            arr->capacity = arr->capacity*2;
            break;
        case 0:
            if (arr->capacity >= init_capacity * 2) {
                arr->data = (char *)DA_REALLOC(allocator, arr->data, size*(arr->capacity/2));
                arr->capacity = arr->capacity/2;
            }
            break;
    }   
}

__attribute((hot,nonnull(2)))
void _array_push(size_t size, dynarray_t *arr, void *elem, size_t init_capacity,void * allocator) {
    if (arr->cnt < arr->capacity) {
        char *ptr = arr->data;
        ptr = ptr + (size * arr->cnt);
        memcpy(ptr, elem, size);
        arr->cnt++;
    } else {
        _array_resize(size, arr, 1, init_capacity,allocator);
        _array_push(size, arr, elem, init_capacity, allocator);
    }
}

__attribute__((nonnull(2)))
void _array_swap_remove(size_t size, dynarray_t *arr, size_t idx, size_t init_capacity, void * allocator) {
    if (arr->cnt > 0) {
        if (idx == arr->cnt-1) {
            arr->cnt--;
        } else if (idx < arr->cnt) {
            char*ptr_dest = arr->data;
            ptr_dest = ptr_dest + (idx * size);
    
            char*ptr_src = arr->data;
            ptr_src = ptr_src + (arr->cnt-1)*size;
    
            memcpy(ptr_dest, ptr_src, size);
            arr->cnt--;
        }
        if (arr->cnt < arr->capacity/3) {
            _array_resize(size, arr, 0, init_capacity,allocator);
        }
    }

}

__attribute__((warn_unused_result,nonnull(2)))
char *_array_pop(size_t size, dynarray_t *arr, size_t init_capacity,void * allocator) {
    char *ptr = NULL;
    if (arr->cnt > 0) {
        ptr = arr->data + size * (arr->cnt - 1);
        arr->cnt--;
        if (arr->cnt < arr->capacity/3) {
            _array_resize(size, arr, 0, init_capacity,allocator);
        }
    }
    return ptr;
}

__attribute__((nonnull(1)))
void _array_free(dynarray_t *arr,void * allocator) {
    DA_FREE(allocator,arr->data);
    DA_FREE(allocator,arr);
}

__attribute__((nonnull(2,4)))
void _array_insert(size_t size, dynarray_t *arr, size_t idx, void *elem, size_t init_capacity,void * allocator) {
    if (idx == arr->cnt) {
        _array_push(size, arr, elem, init_capacity, allocator);
        return;
    }
    if (idx < arr->cnt) {
        if (arr->cnt < arr->capacity) {
            char*ptr = arr->data;
            char*ptr_src = ptr + (idx * size);
            char*ptr_dest = ptr_src + size;
            memcpy(ptr_dest, ptr_src, size * ((arr->cnt)-idx));
            memcpy(ptr_src, elem, size);
            arr->cnt++;
        } else {
            _array_resize(size, arr, 1, init_capacity,allocator);
            _array_insert(size, arr, idx, elem, init_capacity,allocator);
        }
    }
}

__attribute((nonnull(2)))
void _array_remove(size_t size, dynarray_t *arr, size_t idx, size_t init_capacity,void * allocator) {
    if (arr->cnt > 0) {
        if (idx == arr->cnt-1) {
            arr->cnt--;
            return;
        }
        if (idx < arr->cnt-1) {
            char*ptr = arr->data;
            char*ptr_dest = ptr + (idx * size);
            char*ptr_src = ptr_dest + size;
            memcpy(ptr_dest, ptr_src, size * ((arr->cnt-1)-idx));
            arr->cnt--;
        }
        if (arr->cnt < arr->capacity/3) {
            _array_resize(size, arr, 0, init_capacity,allocator);
        }
    }
}

__attribute__((nonnull(2,3)))
void _array_concat(size_t size, dynarray_t *dest, dynarray_t *other, void * allocator) {
    if (dest->capacity > dest->cnt + other->cnt) {

        char *ptr_dest = dest->data + size * dest->cnt;
        char *ptr_src = other->data;
        memcpy(ptr_dest, ptr_src, size * (other->cnt));

        dest->cnt = dest->cnt + other->cnt;
        _array_free(other, allocator);
    } else {
        dest->data = (char *)DA_REALLOC(allocator, dest->data, size*(dest->capacity + other->cnt));
        dest->capacity = dest->capacity + other->cnt;
        _array_concat(size, dest, other, allocator);
    }
}

#endif
// // // // // // // // // // // // // // // 
// Unit tests
#ifdef DYNARRAY_UNIT_TESTS

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>

static int _da_unit_test_part_condition(const int *item, const int *ctx) {
    // DA_UNUSED(ctx);
    return (*item < *ctx);
}

void _da_arr_unit_tests(void) {
    dynarray_t *array = da_arr(int);
    int i, j;
    // assert length of array in two states
    assert(da_arr_len(array) == 0);
    for (i = 0; i < 100; i++) {
        da_arr_push_value(int, array, i);
    }
    assert(da_arr_len(array) == 100);
    // test foreach macro by checking the values of the retrieved items
    da_arr_foreach(int, item, array) {
        assert(*item == j);
        j++;
    }
    // assert if the values in the array are correct by get operation
    for (i = 0; i < 100; i++) {
        assert(*da_arr_get(int, array, i) == i);
    }
    // pop all items from array and assert their values are correct
    for (i = 0; i < 100; i++) {
        assert(*da_arr_pop(int, array) == 99 - i);
    }
    // after popping all items the array should be emtpy
    assert(da_arr_is_empty(array));
    // numbers 0-99 pushed to array
    for (int i = 0; i < 100; i++) {
        da_arr_push_value(int, array, i);
    }
    // assert correctness of values after mapping operation
    da_arr_map(int, array, item, {*item = (*item) * 2;});
    for (i = 0; i < 50; i++) {
        assert(*da_arr_get(int, array, i) == i*2);
    }
    // partition array with even numbers at the front and odd numbers at the back
    printf("before partition\n");
    ptrdiff_t partition_index = da_arr_partition(int, array, _da_unit_test_part_condition);
    printf("after partition\n");
    // assert if partition is successfull the start of the odd numbers must be at index 50
    assert(partition_index == (ptrdiff_t)da_arr_len(array)-1);

    

    for (i = 50; i < 100; i++) {
        da_arr_insert_value(int, array, i, 101);    
    }

    assert(da_arr_len(array) == 150);
    for (i = 50; i < 100; i++) {
        assert(*da_arr_get(int, array, i) == 101);
    }

    da_arr_free(array);

    int tmp = 5;
    array = da_arr_with(int, 10, &tmp);
    assert(arr_len(array) == 10);
    da_arr_foreach(int, item, array) {
        assert(*item == 5);
    }

    da_arr_set_value(int, array, 3, 3);
    assert(*da_arr_get(int, array, 3) == 3);

    da_arr_swap_remove(int, array, 3);
    assert(da_arr_len(array) == 9);
    
    for (i = 0; i < 9; i++) {
        assert(*da_arr_get(int, array, i) == 5);
    }

    da_arr_clear(array);
    // check array_ptr access operation
    da_arr_push_value(int,array,10);
    da_arr_push_value(int,array,5);

    assert(da_arr_ptr(int,array)[0] == 10);

    assert(da_arr_ptr(int,array)[1] == 5);
    
    da_arr_ptr(int,array)[1] = 11;

    assert(da_arr_ptr(int,array)[1] == 11);
    // check insertion_sort 
    // start config of array [10,11,2,5,3,4,9,7]
    da_arr_push_value(int, array, 2);
    da_arr_push_value(int, array, 5);
    da_arr_push_value(int, array, 3);
    da_arr_push_value(int, array, 4);
    da_arr_push_value(int, array, 9);
    da_arr_push_value(int, array, 7);

    da_arr_insertion_sort(int, array, 0, array->cnt, a, b, (*a>*b));
    // end config should be [2,3,4,5,7,9,10,11]
    int validation_array[8] = {2,3,4,5,7,9,10,11};
    for (size_t i = 0; i < arr_len(array); i++) {
        assert(validation_array[i] == *da_arr_get(int,array,i));
    }

    da_arr_set_value(int, array, 0, 15);
    assert(*da_arr_get(int, array, 0) == 15);

    da_arr_free(array);
    array = NULL;
}

#endif

#endif
