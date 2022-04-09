#pragma once

/*
__has_builtin
compiletime_assert
__general_typeof__
*/

#ifndef __has_builtin
#define __has_builtin(x) (0)
#endif

# define __compiletime_assert(condition, msg, prefix, suffix)		\
    do {								\
        /*							\
         * __noreturn is needed to give the compiler enough	\
         * information to avoid certain possibly-uninitialized	\
         * warnings (regardless of the build failing).		\
         */							\
        __attribute__((__noreturn__)) extern void prefix ## suffix(void) __attribute__((__error__(msg)));\
        if (!(condition))					\
        prefix ## suffix();				\
    } while (0)

#define compiletime_assert(condition, msg) \
    _compiletime_assert(condition, msg, __compiletime_assert_, __COUNTER__)

#define _compiletime_assert(condition, msg, prefix, suffix) \
    __compiletime_assert(condition, msg, prefix, suffix)

#define __general_typeof__(x) __typeof__(({__auto_type y=(x); y;}))
