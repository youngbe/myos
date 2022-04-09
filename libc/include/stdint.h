#pragma once

#if defined __x86_64__ && !defined __ILP32__
# define __WORDSIZE 64
#else
# define __WORDSIZE 32
#define __WORDSIZE32_SIZE_ULONG     0
#define __WORDSIZE32_PTRDIFF_LONG   0
#endif

#if __WORDSIZE == 64
typedef signed long int int64_t;
typedef unsigned long int uint64_t;
#else
__extension__ typedef signed long long int int64_t;
__extension__ typedef unsigned long long int uint64_t;
#endif

typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned short int uint16_t;
typedef signed short int int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;



# if __WORDSIZE == 64
#  define INTPTR_MIN        (-9223372036854775807L-1)
#  define INTPTR_MAX        (9223372036854775807L)
#  define UINTPTR_MAX       (18446744073709551615UL)
# else
#  define INTPTR_MIN        (-2147483647-1)
#  define INTPTR_MAX        (2147483647)
#  define UINTPTR_MAX       (4294967295U)
# endif
