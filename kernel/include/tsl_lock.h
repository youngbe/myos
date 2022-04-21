#include <stdint.h>
#include <stdbool.h>

#define LOCKED 1
#define UNLOCKED 0

#define LOCK(x, ... ) \
do \
{ \
    __asm__ volatile \
        ( \
         "movq  $1, %%rax\n" \
         ".Llock%=:\n\t" \
         "xchgq %%rax, %[mutex]\n\t" \
         "testq %%rax, %%rax\n\t" \
         "jnz   .Llock%=" \
         :[mutex]"=m"(x), __VA_ARGS__ \
         : \
         :"rax" \
         ); \
} \
while(false)

#define UNLOCK(x, ... ) \
do \
{ \
    __asm__ volatile \
        ( \
         "movq  $1, %[mutex]" \
         :[mutex]"=m"(x) \
         : __VA_ARGS__ \
         :"rax" \
         ); \
} \
while (false)

typedef uint64_t tsl_mutex;
