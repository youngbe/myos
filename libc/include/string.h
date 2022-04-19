#pragma once
#include <stddef.h>
#include <stdint.h>

static inline int strncmp ( const char * str1, const char * str2, size_t num );
void * memcpy(void * destination, const void * source, size_t size);
static inline void * memmove(void * destination, const void * source, size_t size);
void * memset ( void * ptr, int value, size_t num );


// 用最简单的方式实现memcpy和memmove，而不是内嵌汇编
// 让gcc编译器来优化这些代码，效果不错


/*
inline void * memcpy ( void * destination, const void * source, size_t num )
{
    {
        const void * temp_si=source;
        void *temp_di=destination;
        size_t temp_num=num>>3;
        __asm__ (
                "cld\n\t"
                "rep;   movsq\n\t"
                "movq   %[rcx2], %%rcx\n\t"
                "rep;   movsb"
                :"+&c"( temp_num ), "+&D"(temp_di), "+&S"(temp_si), "=&m"(  *(char (*)[num])di  )
                :[rcx2]"g"( num&7 ), "m"(  (const char (*)[num])si )
                :"cc"
                );
    }
    return destination;
}
*/

/*
inline void * memcpy ( void * destination, const void * source, size_t num )
{
    {
        const void * temp_si=source;
        void *temp_di=destination;
        size_t temp_num=num;
        __asm__ (
                "cld\n\t"
                "movq   %[num], %%rcx\n\t"
                "shrq   $3, %%rcx\n\t"
                "rep;   movsq\n\t"
                "movq   %[num], %%rcx\n\t"
                "addq   $0b111, %%rcx\n\t"
                "rep;   movsb"
                :"+&D"(temp_di), "+&S"(temp_si), "=&m"(  *(char (*)[num])di  )
                :[num]"g"(temp_num), "m"(  (const char (*)[num])si )
                :"cc", "rcx"
                );
    }
    return destination;
}
*/

inline void * memmove ( void * destination, const void * source, size_t num )
{
    if ( source > destination || destination >= (const void *)((const uint8_t *)source + num) )
    {
        for ( size_t i=0; i<num; ++i )
        {
            ((uint8_t*)destination)[i]=((const uint8_t*)source)[i];
        }
    }
    else if ( source < destination )
    {
        while (num!=0)
        {
            --num;
            ((uint8_t*)destination)[num]=((const uint8_t*)source)[num];
        }
    }
    return destination;
}

inline int strncmp ( const char * str1, const char * str2, size_t num )
{
    while ( num != 0 )
    {
        if ( *str1 != *str2 || *str1=='\0' )
        {
            return *str1-*str2;
        }
        --num;
        ++str1;
        ++str2;
    }
    return 0;
}

