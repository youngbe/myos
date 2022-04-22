#pragma once
#include <stdint.h>
#include <stdbool.h>

#define TSL_LOCKED 1
#define TSL_UNLOCKED 0

#define TSL_LOCK(x) \
do \
{ \
    __asm__ volatile \
        ( \
         "movq  $1, %%rax\n" \
         ".Ltsl_lock%=:\n\t" \
         "xchgq %%rax, %[mutex]\n\t" \
         "testq %%rax, %%rax\n\t" \
         "jnz   .Ltsl_lock%=" \
         :[mutex]"+m"(x) \
         : \
         :"rax", "memory" \
         ); \
} \
while(false)

#define TSL_LOCK_CONTENT(x, ... ) \
do \
{ \
    __asm__ volatile \
        ( \
         "movq  $1, %%rax\n" \
         ".Ltsl_lock%=:\n\t" \
         "xchgq %%rax, %[mutex]\n\t" \
         "testq %%rax, %%rax\n\t" \
         "jnz   .Ltsl_lock%=" \
         :[mutex]"+m"(x), __VA_ARGS__ \
         : \
         :"rax" \
         ); \
} \
while(false)

#define TSL_UNLOCK(x) \
do \
{ \
    __asm__ volatile \
        ( \
         "movq  $1, %[mutex]" \
         :[mutex]"=m"(x) \
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
         "movq  $1, %[mutex]" \
         :[mutex]"=m"(x) \
         : __VA_ARGS__ \
         : \
         ); \
} \
while (false)

typedef uint64_t tsl_mutex;
