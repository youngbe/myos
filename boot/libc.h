#pragma once

#if defined __x86_64__ && !defined __ILP32__
typedef unsigned long int uint64_t;
typedef signed long int int64_t;
#else
typedef unsigned long long int uint64_t;
typedef signed long long int int64_t;
#endif
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned short int uint16_t;
typedef signed short int int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
#if defined __x86_64__ && !defined __ILP32__
typedef uint64_t size_t;
typedef int64_t ssize_t;
#else
typedef uint32_t size_t;
typedef int32_t ssize_t;
#endif

typedef _Bool bool;
#define true ((_Bool)+1u)
#define false ((_Bool)+0u)

#define NULL ((void *)0)

static inline int strncmp ( const char * str1, const char * str2, size_t num );
static inline void * memcpy(void * destination, const void * source, size_t size);
static inline void * memmove(void * destination, const void * source, size_t size);
static inline void * memset ( void * ptr, int value, size_t num );
// 实际上是冒泡排序
static inline void qsort( void* base, size_t num, size_t width, int(*compare)(const void*,const void*) );

inline void qsort( void* base, size_t num, size_t width, int(*compare)(const void*,const void*) )
{
    for ( size_t i=1; i<num; ++i )
    {
        for ( size_t i2=0; i2<num-i; ++i2 )
        {
            if ( compare( (const uint8_t *)base+width*i2, (const uint8_t *)base+width*(i2+1) ) > 0 )
            {
                uint8_t temp[width];
                memcpy(temp, (uint8_t *)base+width*i2, width );
                memcpy((uint8_t *)base+width*i2, (uint8_t *)base+width*(i2+1) , width);
                memcpy((uint8_t *)base+width*(i2+1), temp, width);
            }
        }
    }
}

inline void * memcpy ( void * destination, const void * source, size_t num )
{
    for ( size_t i=0; i<num; ++i )
    {
        ((uint8_t*)destination)[i]=((const uint8_t*)source)[i];
    }
    return destination;
}

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

inline void * memset ( void * ptr, int value, size_t num )
{
    while ( num != 0 )
    {
        --num;
        ((uint8_t *)ptr)[num]=(uint8_t)value;
    }
    return ptr;
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
