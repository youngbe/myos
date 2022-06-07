#include "public.h"

#include <stddef.h>
#include <stdint.h>

typedef struct Memory_Block Memory_Block;
__attribute__((noreturn))
void init(const Memory_Block *const blocks, const size_t blocks_num);


// 设置section为.text.entry_point，与ld脚本配合，使得_start函数在内核文件的开头
extern "C"
__attribute__((section(".text.entry_point"), noreturn))
void _start(const Memory_Block *const blocks, const size_t blocks_num)
{
    // 重定位
    {
        typedef struct __attribute__((packed)) Elf64_Rela
        {
            uint64_t offset;
            uint64_t info;
            uint64_t append;
        } Elf64_Rela;
        extern const Elf64_Rela __rela_dyn_start[];
        extern const Elf64_Rela __rela_dyn_end[];
        const Elf64_Rela* rela=__rela_dyn_start;
        while ( rela != __rela_dyn_end )
        {
            if ( rela->info != 8 )
            {
                kernel_abort("unknown relocation info!");
            }
            *(uint64_t *)((uintptr_t)_start+rela->offset)=(uint64_t)((uintptr_t)_start+rela->append);
            ++rela;
        }
        __asm__ volatile ("":::"memory");
    }
    // 运行init_array代码
    {
        extern void (*const __init_array_start[])();
        extern void (*const __init_array_end[])();
        void (*const*fun)()=__init_array_start;
        while ( fun != __init_array_end )
        {
            (*fun)();
            ++fun;
        }
    }
    init(blocks, blocks_num);
}
