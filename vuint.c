#include "vuint.h"
#include <limits.h>

#define UNTAG_PTR(p) ((void*)((uintptr_t)p & ~3))

// returns carry (0 .. 1)
// result wraps around
// @param dir   0 if little endian, non-0 if big
static int carry_one(unsigned * a, size_t a_remain, int dir) {
    unsigned short a_large = *a;
    ++a_large;
    *a = *a + 1;
    if (a_large > UCHAR_MAX)
        return --a_remain ? 1 : carry_one(a + (dir ? 1 : -1), a_remain, dir);
    return 0;
}

// mutates a->data
static int vuint_carry_one(vuint* a, size_t a_offset) {
    return carry_one(a->item(a), a->data_size - a_offset, (uintptr_t)a->tagged_data & VUINT_BIG_ENDIAN);
}

// returns carry or overflow (0 .. 1)
// -1 reserved may be used for special errors in future
static int vuint_operator(vuint* a, void* b, size_t b_cb, vuint_flags b_info, vuint_oper operator) 
{
    unsigned char* a_iter = (uintptr_t)a->tagged_data | VUINT_BIG_ENDIAN
                            ? a->item(a)
                            : (void*)((char*)a->item(a) + a->data_size - 1);
    unsigned char* b_iter = b_info | VUINT_BIG_ENDIAN
                            ? b
                            : (void*)((char*)b + b_cb - 1);
    int    ret_carry = 0;
    size_t byte_index = 0;
    // If b == NULL then single operand
    while (byte_index < a->data_size && (!b ? true : byte_index < b_cb)) {
        unsigned short _a_iter_large = *a_iter;
        bool           _carry = false;
        switch (operator) {
            case ADD:
                _a_iter_large += *b_iter;
                *a_iter = (unsigned char)_a_iter_large;
                break;
            case SUB:
                _a_iter_large -= *b_iter;
                *a_iter = (unsigned char)_a_iter_large;
                break;
            case MUL:
                _a_iter_large *= *b_iter;
                *a_iter = (unsigned char)_a_iter_large;
                break;
            case DIV:
                _a_iter_large /= *b_iter;
                *a_iter = (unsigned char)_a_iter_large;
            case AND:
                *a_iter &= *b_iter;
                break;
            case OR:
                *a_iter |= *b_iter;
            case XOR:
                *a_iter ^= *b_iter;
                break;
            case NOT:
                *a_iter = ~*a_iter;
                break;
        }
        if (_a_iter_large > UCHAR_MAX)
            _carry = true;
        
        a_iter += (uintptr_t)a->tagged_data | VUINT_BIG_ENDIAN ? 1 : -1;
        b_iter += b_info | VUINT_BIG_ENDIAN ? 1 : -1;

        if (_carry)
            if (vuint_carry_one(a, byte_index))
                ret_carry = 1;

        ++byte_index;
    }
    return ret_carry;
}

static void* vuint_item(void* self) {
    return UNTAG_PTR(((vuint*)self)->tagged_data);
}

static bool vuint_is_big_endian(void* self) {
    return (uintptr_t)((vuint*)self)->tagged_data & VUINT_BIG_ENDIAN;
}

vuint* vuint_ctor(vuint* a, size_t cb, vuint_flags info) 
{
    a->data_size = cb;
    
    if (a->data_size <= _vuint_storage) {
        a->tagged_data = (void*)a->small_storage;
    } else {
        a->tagged_data = malloc(cb);
        // todo malloc check
        a->tagged_data = (void*)((uintptr_t)a->tagged_data | VUINT_EXTERNAL_STORAGE);
    }
    a->tagged_data =  (void*)((uintptr_t)a->tagged_data | (info & VUINT_BIG_ENDIAN));
    a->item = vuint_item;
    a->is_big_endian = vuint_is_big_endian;
    memset(a->item(a), 0, a->data_size);
    return a;
}

void vuint_dtor(vuint* a)
{
    if ((uintptr_t)a->tagged_data & VUINT_EXTERNAL_STORAGE)
        free(a->item(a));
    memset(a, 0, sizeof(*a));
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
    vuint_operator(a, NULL, 0, 0, NOT);
}
