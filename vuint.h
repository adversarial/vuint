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
#include <stdint.h>
#include <stdbool.h>
// WARNING nonstandard ptr math by conversion to uintptr_t

#define RELEASE_BUILD 0

#if RELEASE_BUILD
#   define PRIVATE_VAR(name, num) Reserved##num
#   define CLIENT_CONST const
#else
#   define PRIVATE_VAR(name, num) name
#   define CLIENT_CONST
#endif

typedef enum { ADD = '+', SUB = '-', MUL = '*', DIV = '/', AND = '&', OR = '|',
                XOR = '^', NOT = '~' } vuint_oper;

typedef void* vuint_tagptr;

// size of internal storage in struct ( do not change without rebuilding lib )
static const int _vuint_storage = 8;

typedef unsigned int vuint_flags;
const vuint_flags PRIVATE_VAR(VUINT_EXTERNAL_STORAGE, 1) = 1 << 0,
                  VUINT_BIG_ENDIAN                       = 1 << 1;

typedef struct {
    // Tagged 1 << 0 as small_storage ptr
    // Untag before dereferencing (x & ~3)
    // Reserved1 (public name)
    CLIENT_CONST vuint_tagptr*  PRIVATE_VAR(data, 1);
    CLIENT_CONST size_t       data_size;
    // A small block inside holds most data
    // Reserved2 (public name)
    CLIENT_CONST unsigned char PRIVATE_VAR(small_storage, 2)[_vuint_storage];
    // Returns a pointer to the array
    // Be aware of endianness 
    void* (*array)();
    bool (*is_big_endian)();
} vuint;

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
#undef PRIVATE_VAR
