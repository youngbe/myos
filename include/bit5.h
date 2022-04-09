#pragma once
#include <tools.h>
#include <stdint.h>
#include <stdbool.h>

// Support all type even float, double or struct
#define GET_BITS_LOW(var, bit_nums) ({ \
    __general_typeof__(var) ret; \
    if ( sizeof(var) == 1 ) \
    { \
        union __attribute__((packed)) \
        { \
            uint8_t pad; \
            __general_typeof__(var) _var; \
        } temp; \
        temp._var=(var); \
        __auto_type temp_bit_nums=(bit_nums); \
        if ( temp_bit_nums >= 8 ) \
        { \
            compiletime_assert(false, "GET_BITS_LOW:Out of range or unknown bit_nums when compiling"); \
        } \
        temp.pad&= (((uint8_t)1)<<temp_bit_nums) - ((uint8_t)1); \
        ret=temp._var; \
    } \
    else if ( sizeof(var) == 2 ) \
    { \
        union __attribute__((packed)) \
        { \
            uint16_t pad; \
            __general_typeof__(var) _var; \
        } temp; \
        temp._var=(var); \
        __auto_type temp_bit_nums=(bit_nums); \
        if ( temp_bit_nums >= 16 ) \
        { \
            compiletime_assert(false, "GET_BITS_LOW:Out of range or unknown bit_nums when compiling"); \
        } \
        temp.pad&= (((uint16_t)1)<<temp_bit_nums) - ((uint16_t)1); \
        ret=temp._var; \
    } \
    else if ( sizeof(var) == 4 ) \
    { \
        union __attribute__((packed)) \
        { \
            uint32_t pad; \
            __general_typeof__(var) _var; \
        } temp; \
        temp._var=(var); \
        __auto_type temp_bit_nums=(bit_nums); \
        if ( temp_bit_nums >= 32 ) \
        { \
            compiletime_assert(false, "GET_BITS_LOW:Out of range or unknown bit_nums when compiling"); \
        } \
        temp.pad&= (((uint32_t)1)<<temp_bit_nums) - ((uint32_t)1); \
        ret=temp._var; \
    } \
    else if ( sizeof(var) == 8 ) \
    { \
        union __attribute__((packed)) \
        { \
            uint64_t pad; \
            __general_typeof__(var) _var; \
        } temp; \
        temp._var=(var); \
        __auto_type temp_bit_nums=(bit_nums); \
        if ( temp_bit_nums >= 64 ) \
        { \
            compiletime_assert(false, "GET_BITS_LOW:Out of range or unknown bit_nums when compiling"); \
        } \
        temp.pad&= (((uint64_t)1)<<temp_bit_nums) - ((uint64_t)1); \
        ret=temp._var; \
    } \
    else \
    { \
        compiletime_assert(false, "GET_BITS_LOW:Unsupport size!"); \
    } \
    ret; \
})

#define REMOVE_BITS_LOW(var, bit_nums) ({ \
    __general_typeof__(var) ret; \
    if ( sizeof(var) == 1 ) \
    { \
        union __attribute__((packed)) \
        { \
            uint8_t pad; \
            __general_typeof__(var) _var; \
        } temp; \
        temp._var=(var); \
        __auto_type temp_bit_nums=(bit_nums); \
        if ( temp_bit_nums >= 8 ) \
        { \
            compiletime_assert(false, "REMOVE_BITS_LOW:Out of range or unknown bit_nums when compiling"); \
        } \
        temp.pad&= ((uint8_t)-1)<<temp_bit_nums; \
        ret=temp._var; \
    } \
    else if ( sizeof(var) == 2 ) \
    { \
        union __attribute__((packed)) \
        { \
            uint16_t pad; \
            __general_typeof__(var) _var; \
        } temp; \
        temp._var=(var); \
        __auto_type temp_bit_nums=(bit_nums); \
        if ( temp_bit_nums >= 16 ) \
        { \
            compiletime_assert(false, "REMOVE_BITS_LOW:Out of range or unknown bit_nums when compiling"); \
        } \
        temp.pad&= ((uint16_t)-1)<<temp_bit_nums; \
        ret=temp._var; \
    } \
    else if ( sizeof(var) == 4 ) \
    { \
        union __attribute__((packed)) \
        { \
            uint32_t pad; \
            __general_typeof__(var) _var; \
        } temp; \
        temp._var=(var); \
        __auto_type temp_bit_nums=(bit_nums); \
        if ( temp_bit_nums >= 32 ) \
        { \
            compiletime_assert(false, "REMOVE_BITS_LOW:Out of range or unknown bit_nums when compiling"); \
        } \
        temp.pad&= ((uint32_t)-1)<<temp_bit_nums; \
        ret=temp._var; \
    } \
    else if ( sizeof(var) == 8 ) \
    { \
        union __attribute__((packed)) \
        { \
            uint64_t pad; \
            __general_typeof__(var) _var; \
        } temp; \
        temp._var=(var); \
        __auto_type temp_bit_nums=(bit_nums); \
        if ( temp_bit_nums >= 64 ) \
        { \
            compiletime_assert(false, "REMOVE_BITS_LOW:Out of range or unknown bit_nums when compiling"); \
        } \
        temp.pad&= ((uint64_t)-1)<<temp_bit_nums; \
        ret=temp._var; \
    } \
    else \
    { \
        compiletime_assert(false, "REMOVE_BITS_LOW:Unsupport size!"); \
    } \
    ret; \
})
