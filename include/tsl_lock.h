#pragma once
#include <stdint.h>
#include <stdbool.h>

#define TSL_LOCKED 1
#define TSL_UNLOCKED 0

#define TSL_LOCK(x) \
do \
{ \
    uint64_t temp=TSL_LOCKED; \
    uint64_t temp2; \
    __asm__ volatile \
        ( \
         ".Ltsl_lock%=:\n\t" \
         "xorq  %%rax, %%rax\n\t" \
         "cmpxchgq %[temp], %[mutex]\n\t" \
         "jne   .Ltsl_lock%=" \
         :[mutex]"+m"(*(volatile uint64_t *)&x), [temp2]"=&a"(temp2) \
         :[temp]"r"(temp) \
         :"memory", "cc" \
         ); \
    if ( temp2 != 0 ) \
        __builtin_unreachable();\
} \
while(false)

#define TSL_LOCK_CONTENT(x, ... ) \
do \
{ \
    uint64_t temp=TSL_LOCKED; \
    uint64_t temp2; \
    __asm__ volatile \
        ( \
         ".Ltsl_lock%=:\n\t" \
         "xorq  %%rax, %%rax\n\t" \
         "cmpxchgq %[temp], %[mutex]\n\t" \
         "jne   .Ltsl_lock%=" \
         :[mutex]"+m"(*(volatile uint64_t *)&x), [temp2]"=&a"(temp2), __VA_ARGS__ \
         :[temp]"r"(temp) \
         :"memory", "cc" \
         ); \
    if ( temp2 != 0 ) \
        __builtin_unreachable();\
} \
while(false)

#define TSL_UNLOCK(x) \
do \
{ \
    __asm__ volatile \
        ( \
         "movq  $0, %[mutex]" \
         :[mutex]"=m"(*(volatile uint64_t *)&x) \
         : \
         :"memory" \
         ); \
} \
while (false)

#define TSL_UNLOCK_CONTENT(x, ... ) \
do \
{ \
    __asm__ volatile \
        ( \
         "movq  $0, %[mutex]" \
         :[mutex]"=m"(*(volatile uint64_t *)&x) \
         : __VA_ARGS__ \
         : \
         ); \
} \
while (false)

#define CLI_TSL_LOCK(x) \
do \
{ \
    uint64_t temp=TSL_LOCKED; \
    uint64_t temp2; \
    __asm__ volatile \
        ( \
         "cli\n" \
         ".Ltsl_lock%=:\n\t" \
         "xorq  %%rax, %%rax\n\t" \
         "cmpxchgq %[temp], %[mutex]\n\t" \
         "jne   .Ltsl_lock%=" \
         :[mutex]"+m"(*(volatile uint64_t *)&x), [temp2]"=&a"(temp2) \
         :[temp]"r"(temp) \
         :"memory", "cc" \
         ); \
    if ( temp2 != 0 ) \
        __builtin_unreachable();\
} \
while(false)

#define CLI_TSL_LOCK_CONTENT(x, ... ) \
do \
{ \
    uint64_t temp=TSL_LOCKED; \
    uint64_t temp2; \
    __asm__ volatile \
        ( \
         "cli\n" \
         ".Ltsl_lock%=:\n\t" \
         "xorq  %%rax, %%rax\n\t" \
         "cmpxchgq %[temp], %[mutex]\n\t" \
         "jne   .Ltsl_lock%=" \
         :[mutex]"+m"(*(volatile uint64_t *)&x), [temp2]"=&a"(temp2), __VA_ARGS__ \
         :[temp]"r"(temp) \
         :"memory", "cc" \
         ); \
    if ( temp2 != 0 ) \
        __builtin_unreachable();\
} \
while(false)

#define STI_TSL_UNLOCK(x) \
do \
{ \
    __asm__ volatile \
        ( \
         "movq  $0, %[mutex]\n\t" \
         "sti" \
         :[mutex]"=m"(*(volatile uint64_t *)&x) \
         : \
         :"memory" \
         ); \
} \
while (false)

#define STI_TSL_UNLOCK_CONTENT(x, ... ) \
do \
{ \
    __asm__ volatile \
        ( \
         "movq  $0, %[mutex]\n\t" \
         "sti" \
         :[mutex]"=m"(*(volatile uint64_t *)&x) \
         : __VA_ARGS__ \
         : \
         ); \
} \
while (false)

typedef uint64_t tsl_mutex;
