#pragma once
#include <stddef.h>
#include <stdint.h>

static inline int strncmp ( const char * str1, const char * str2, size_t num );
/*
 * 根据文档，gcc编译器需要提供memcpy, memmove, memset, memcmp 的额外实现
 * https://gcc.gnu.org/onlinedocs/gcc/Standards.html
 * 这里预编译好了 memcpy.s memmove.s memset.s memcmp.s
static inline void * memcpy(void * destination, const void * source, size_t size);
static inline void * memmove(void * destination, const void * source, size_t size);
static inline void * memset ( void * ptr, int value, size_t num );
static inline int memcmp(const void *str1, const void *str2, size_t n);
*/
void * memcpy(void * destination, const void * source, size_t size);
void * memmove(void * destination, const void * source, size_t size);
void * memset ( void * ptr, int value, size_t num );
int memcmp(const void *str1, const void *str2, size_t n);



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
