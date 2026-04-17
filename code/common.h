

// Aleksey Yevtushenko
// February 13, 2022

#if !defined(COMMON_H)
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#define internal static
#define global static
#define local_persist static

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef s32 b32;
typedef float f32;
typedef double f64;

typedef size_t mem_index;

#define Assert(expr) assert((expr))
#define U64_MAX UINT64_MAX
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Min(a, b) ((a) < (b) ? (a) : (b))
#define SizeOf(item) ((char *)(&item + 1) - (char *)(&item)) // FixMe: Doesn't work with types.
#define OffsetOf(type, member) ((mem_index)(&((type *)0)->member))

typedef struct DynamicArray{
    mem_index len;
    mem_index cap;
    char arr[1];
}DynamicArray;

#define DynamicArray__header(array) ((DynamicArray *)((char *)(array) - OffsetOf(DynamicArray, arr)))
#define DynamicArray__doesfit(array, count) ((DynamicArray__len(array)+(count)) <= DynamicArray__cap(array))
#define DynamicArray__fit(array, count) (DynamicArray__doesfit(array, count) ? 0 : \
                                         ((array)=DynamicArray__grow(array, (DynamicArray__len(array)+(count)), SizeOf(*(array)))))

#define DynamicArray__push(array, item) (DynamicArray__fit(array, 1),   \
                                         (array)[DynamicArray__len(array)] = (item), \
                                         DynamicArray__header(array)->len += 1)

#define DynamicArray__len(array) ((array) ? (DynamicArray__header(array)->len) : 0)
#define DynamicArray__cap(array) ((array) ? (DynamicArray__header(array)->cap) : 0)
#define DynamicArray__free(array) ((array) ? (free(DynamicArray__header(array)), array = 0) : 0)
#define DynamicArray__pop(array) // TODO: Implement.

static void *DynamicArray__grow(void *array, mem_index requested_size, mem_index item_size){
    mem_index updated_cap = Max(requested_size, (2 * DynamicArray__cap(array) + 1));
    mem_index size = (updated_cap * item_size + OffsetOf(DynamicArray, arr));
    DynamicArray *darr = 0;
    if(array)
        darr = (DynamicArray *)realloc(DynamicArray__header(array), size);
    else{
        darr = (DynamicArray *)malloc(size);
        darr->len = 0;
    }
    darr->cap = updated_cap;
    return((void *)darr->arr);
}

#define darr_len(array) DynamicArray__len(array)
#define darr_cap(array) DynamicArray__cap(array)
#define darr_push(array, item) DynamicArray__push(array, item)
// #define darr_pop(array) DynamicArray__pop(array)
#define darr_free(array) DynamicArray__free(array)

#include "string_guard.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"

#endif // COMMON_H
