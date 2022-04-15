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

/* Types for `void *' pointers.  */
#if __WORDSIZE == 64
# ifndef __intptr_t_defined
typedef long int        intptr_t;
#  define __intptr_t_defined
# endif
typedef unsigned long int   uintptr_t;
#else
# ifndef __intptr_t_defined
typedef int         intptr_t;
#  define __intptr_t_defined
# endif
typedef unsigned int        uintptr_t;
#endif

/* Fast types.  */

/* Signed.  */
typedef signed char		int_fast8_t;
#if __WORDSIZE == 64
typedef long int		int_fast16_t;
typedef long int		int_fast32_t;
typedef long int		int_fast64_t;
#else
typedef int			int_fast16_t;
typedef int			int_fast32_t;
__extension__
typedef long long int		int_fast64_t;
#endif

/* Unsigned.  */
typedef unsigned char		uint_fast8_t;
#if __WORDSIZE == 64
typedef unsigned long int	uint_fast16_t;
typedef unsigned long int	uint_fast32_t;
typedef unsigned long int	uint_fast64_t;
#else
typedef unsigned int		uint_fast16_t;
typedef unsigned int		uint_fast32_t;
__extension__
typedef unsigned long long int	uint_fast64_t;
#endif
