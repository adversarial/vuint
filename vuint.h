/*
==================================================================
vuint - variable sized unsigned int operations
Copyright (C) 2015 - vividrev -- see \LICENSE.TXT
==================================================================

  > vuint.h -- C/C++ obj header 
  > vuint.c -- obj source
  > LICENSE -- MIT license

Project hosted at http://github.com/vividrev/

 -- vividrev (vividrev@outlook.com) :: netrev.0fees.us
 
==================================================================
*/

#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
// WARNING nonstandard ptr math by conversion to uintptr_t

typedef enum { ADD, SUB, MUL, DIV, AND, OR, XOR, NOT } vuint_oper;

typedef int vuint_flags;

// size of internal storage in struct ( do not change without rebuilding lib )
#define _vuint_storage 8

typedef struct {
    // Tagged 1 << 0 as small_storage ptr
    // Untag before dereferencing (x & ~3)
    // Reserved1 (public name)
    void*        tagged_data;
    size_t       data_size;
    // A small block inside holds most data
    unsigned char small_storage[_vuint_storage];
    // Returns a pointer to the array
    // Be aware of endianness 
    // Untags data for user
    void* (*item)(void* self);
    bool (*is_big_endian)(void* self);
} vuint;

// passing undefined values will have no effect
const vuint_flags VUINT_EXTERNAL_STORAGE = 1,
                  VUINT_BIG_ENDIAN       = 2;

#ifdef __cplusplus
extern 'C' {
#endif

// Returns a pointer to an array of data_size bits
// or NULL if version is incompatible
// Array is in little endian, or BIG_ENDIAN if true (non-default)
vuint* vuint_ctor(vuint* a, size_t cb, vuint_flags info);
void vuint_dtor(vuint* a);
// Returns carry
// res may be equal to a or b
bool vuint_add(vuint* a, void* b, size_t b_cb, vuint_flags b_info);
bool vuint_sub(vuint* a, void* b, size_t b_cb, vuint_flags b_info);
bool vuint_mul(vuint* a, void* b, size_t b_cb, vuint_flags b_info);
bool vuint_div(vuint* a, void* b, size_t b_cb, vuint_flags b_info);
bool vuint_and(vuint* a, void* b, size_t b_cb, vuint_flags b_info);
bool vuint_or(vuint* a, void* b, size_t b_cb, vuint_flags b_info);
bool vuint_xor(vuint* a, void* b, size_t b_cb, vuint_flags b_info);
void vuint_not(vuint* a);

#ifdef __cplusplus
}
#endif
