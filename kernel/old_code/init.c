#include <init.h>

extern int _start[];
extern int __kernel_end[];

#define MAX_BLOCKS_NUM 512
void init(size_t block_nums, Memory_Block *blocks)
{
    if ( GET_BITS_LOW((uint64_t)_start, 4) != 0 )
    {
        // 内核加载位置aligned不足
        error();
    }


    Memory_Block usable_blocks[MAX_BLOCKS_NUM];
    size_t usable_blocks_num=0;
    size_t free_pages_num=0;
    for ( size_t i=0; i<block_nums; ++i )
    {
        if ( blocks[i].type != 1 )
        {
            continue;
        }
        uint64_t start=blocks[i].base;
        uint64_t end=blocks[i].base+blocks[i].size;
        // <16M or >256T
        if ( end < (((uint64_t)18)<<20) || start >= ((uint64_t)1<<48) )
        {
            continue;
        }
        if ( start <= (((uint64_t)16)<<20) )
        {
            start=(((uint64_t)16)<<20);
        }
        else
        {
            // 向上取整2M
            start=REMOVE_BITS_LOW(start-1, 21)+ (((uint64_t)2)<<20);
        }
        if ( end > ((uint64_t)1<<48) )
        {
            end = (uint64_t)1<<48;
        }
        else
        {
            // 向下取整2M
            end=REMOVE_BITS_LOW(end, 21);
        }
        if ( end > start )
        {
            if ( usable_blocks_num == MAX_BLOCKS_NUM )
            {
                error();
            }
            usable_blocks[usable_blocks_num].base=start;
            usable_blocks[usable_blocks_num].size=end-start;
            usable_blocks[usable_blocks_num].type=1;
            usable_blocks_num++;
            free_pages_num+=(end-start)>>21;
        }
    }


    // 插入内核代码
    for ( size_t i=0; i<usable_blocks_num; ++i )
    {
        if ( usable_blocks[i].start <= (uint64_t)_start && (uint64_t)__kernel_end <= usable_blocks[i].start + usable_blocks[i].size )
        {
            if ( (uint64_t)_start <= usable_blocks[i].start + 128 )
            {
                usable_blocks[i].type=6;
                if ( (uint64_t)__kernel_end < usable_blocks[i].base + usable_blocks[i].size - 128 )
                {
                    if ( usable_blocks_num == MAX_BLOCKS_NUM )
                    {
                        error();
                    }
                    // insert in usable_blocks[i+1]
                    memmove(&usable_blocks[i+2], &usable_blocks[i+1], sizeof(Memory_Block)*(usable_blocks_num-i-1));
                    ++usable_blocks_num;
                    usable_blocks[i+1].base=(uint64_t)__kernel_end;
                    usable_blocks[i+1].size=usable_blocks[i].base+usable_blocks[i].size-(uint64_t)__kernel_end;
                    usable_blocks[i+1].type=1;
                    usable_blocks[i].size=(uint64_t)__kernel_end-usable_blocks[i].base;
                }
            }
            else
            {
                if ( (uint64_t)__kernel_end >= usable_blocks[i].base + usable_blocks[i].size - 128 )
                {
                    if ( usable_blocks_num == MAX_BLOCKS_NUM )
                    {
                        error();
                    }
                    // insert in usable_blocks[i+1]
                    memmove(&usable_blocks[i+2], &usable_blocks[i+1], sizeof(Memory_Block)*(usable_blocks_num-i-1));
                    ++usable_blocks_num;
                    usable_blocks[i+1].base=(uint64_t)_start;
                    usable_blocks[i+1].size=usable_blocks[i].base+usable_blocks[i].size-(uint64_t)_start;
                    usable_blocks[i+1].type=6;
                    usable_blocks[i].size=(uint64_t)_start-usable_blocks[i].base;
                }
                else
                {
                    if ( usable_blocks_num +1 >= MAX_BLOCKS_NUM )
                    {
                        error();
                    }
                    // insert 2 in usable_blocks[i+1]
                    memmove(&usable_blocks[i+3], &usable_blocks[i+1], sizeof(Memory_Block)*(usable_blocks_num-i-1));
                    usable_blocks_num+=2;
                    usable_blocks[i+1].base=(uint64_t)_start;
                    usable_blocks[i+1].size=(uint64_t)__kernel_end-(uint64_t)_start;
                    usable_blocks[i+1].type=6;
                    usable_blocks[i+2].base=(uint64_t)__kernel_end;
                    usable_blocks[i+2].size=usable_blocks[i].base+usable_blocks[i].size-(uint64_t)__kernel_end;
                    usable_blocks[i+2].type=1;
                    usable_blocks[i].size=(uint64_t)_start-usable_blocks[i].base;
                }
            }
            goto label1;
        }
    }
    // 内核代码和不可用页交错
    error();
label1:


    // 分配 TSS,GDT，线程表、空闲栈 的空间


    size_t free_num=510;
    struct
    {
        // stack[0]== stack.size
        // stack is stack[ 1 ... stack[0] ]
        uint32_t stack[512];

        struct list_nhlist free_list;
        // 页表中的有效条目数
        uint32_t num[512];
    }
    // 初始化页表
    

    // gdt idt tss初始化

}
