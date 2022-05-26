#include "public.h"
#include "terminal.h"

#include <stdint.h>
#include <stddef.h>

typedef struct Memory_Block Memory_Block;
struct __attribute__((packed)) Memory_Block
{
    uint64_t base;
    uint64_t size;
    uint32_t type;
};

typedef struct Memory_Block Memory_Block;
__attribute__((noreturn))
void init(const Memory_Block *const blocks, const size_t blocks_num)
{
    tputs("init end!\n");
    __asm__ volatile("":::"memory");
    while (1);
}
