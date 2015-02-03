#include "vuint.h"
#include <limits.h>

#define UNTAG_PTR(p) ((void*)((uintptr_t)p & ~3))
vuint* vuint_ctor(vuint* a, size_t cb, vuint_flags info) 
{
    a->data_size = cb;
    
    if (data_size <= _vuint_storage) {
        a->data = (void*)a->small_storage;
    } else {
        a->data = malloc(cb);
        // todo malloc check
        a->data = (void*)((uintptr_t)a->data | VUINT_EXTERNAL_STORAGE);
    }
    a->data =  (void*)((uintptr_t)a->data | (info & VUINT_BIG_ENDIAN));
    memset(UNTAG_PTR(a->data), 0, a->data_size);
    return a;
}

void vuint_dtor(vuint* a)
{
    if ((uintptr_t)a->data & VUINT_EXTERNAL_STORAGE)
        free(UNTAG_PTR(a->data));
    memset(a, 0, sizeof(*a)):
}

bool vuint_add(vuint* a, void* b, size_t b_cb, vuint_flags b_info)
{
    return (bool)vuint_operator(a, b, b_cb, b_info, ADD);
}

bool vuint_sub(vuint* a, void* b, size_t b_cb, vuint_flags b_info) 
{
    return (bool)vuint_operator(a, b, b_cb, b_info, SUB);
}
bool vuint_mul(vuint* a, void* b, size_t b_cb, vuint_flags b_info) 
{
    return (bool)vuint_operator(a, b, b_cb, b_info, MUL);
}
bool vuint_div(vuint* a, void* b, size_t b_cb, vuint_flags b_info) 
{
    return (bool)vuint_operator(a, b, b_cb, b_info, DIV);
}
bool vuint_and(vuint* a, void* b, size_t b_cb, vuint_flags b_info) 
{
    return (bool)vuint_operator(a, b, b_cb, b_info, AND);
}
bool vuint_or(vuint* a, void* b, size_t b_cb, vuint_flags b_info) 
{
    return (bool)vuint_operator(a, b, b_cb, b_info, OR);
}
bool vuint_xor(vuint* a, void* b, size_t b_cb, vuint_flags b_info) 
{
    return (bool)vuint_operator(a, b, b_cb, b_info, XOR);
}
void vuint_not(vuint* a)
{
    return (bool)vuint_operator(a, NULL, 0, 0, NOT);
}

int vuint_operator(vuint* a, void* b, size_t b_cb, vuint_flags b_info, vuint_oper operator) 
{
    unsigned char* a_iter = (uintptr_t)a->data & VUINT_BIG_ENDIAN 
                            ? UNTAG_PTR(a->data) 
                            : (void*)((char*)UNTAG_PTR(a->data) + a->data_size - 1);
    unsigned char* b_iter = b_info | VUINT_BIG_ENDIAN
                            ? b
                            : (void*)((char*)b + b_cb - 1);
    int    ret_carry = 0;
    size_t byte_index = 0;
    // If b == NULL then single operand
    while (byte_index < a->data_size && (!b ? true : byte_index < b_cb)) {
        // Null checks for unused operands
        unsigned short _a_iter_large = a_iter ? *a_iter : 0,
                       _b_iter_large = b_iter ? *b_iter : 0;
        bool           _carry = false;
        switch (operator) {
            case ADD:
                _a_iter_large += *b_iter;
                if (_a_iter_large > UCHAR_MAX)
                    _carry = true;
                *a_iter = (unsigned char)_a_iter_large;
                break;
            case SUB:
                _a_iter_large -= *b_iter;
                if (_a_iter_large > UCHAR_MAX)
                    _carry = true;
                *a_iter = (unsigned char)_a_iter_large;
                break;
            case MUL:
                _a_iter_large *= *b_iter;
                if (_a_iter_large > UCHAR_MAX)
                    _carry = true;
                *a_iter = (unsigned char)_a_iter_large;
                break;
            case DIV:
                _a_iter_large /= *b_iter;
                if (_a_iter_large > UCHAR_MAX)
                    _carry = true;
                *a_iter = (unsigned char)_a_iter_large;
            case AND:
                *a_iter &= b_iter;
                break;
            case OR:
                *a_iter |= b_iter;
            case XOR:
                *a_iter ^= b_iter;
                break;
            case NOT:
                *a_iter = ~*a_iter;
                break;
        }
        ++byte_index;
        
        if ((uintptr_t)a->data | VUINT_BIG_ENDIAN) 
            ++a_iter;
        else --a_iter;
        if (b_info | VUINT_BIG_ENDIAN)
            ++b_iter;
        else --b_iter;
        
        size_t _a_carry_index = byte_index,
               _a_offest = 0;
        while (_carry && _a_carry_index < a->data_size) {
            // add and check for carry
            _a_iter_large = *(a_iter + _a_offset);
            ++_a_iter_large;
            if (_a_iter_large > UCHAR_MAX)
                _carry = true;  
            // special case, size exceeded
            if (_a_carry_index >= a->data_size && _carry)
                ret_carry = 1;
            // advance depending on endianness
            if ((uintptr_t)a->data | VUINT_BIG_ENDIAN) 
                ++_a_offset;
            else --_a_offset;
            ++_a_carry_index;
        }
    }
    return ret_carry;
}

