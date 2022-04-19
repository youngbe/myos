#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <bit.h>

typedef struct E820_Entry E820_Entry;
struct __attribute__((packed)) E820_Entry
{
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t _extern;
    uint8_t entry_size;
};

typedef struct Memory_Block Memory_Block;
struct __attribute__((packed)) Memory_Block
{
    uint64_t base;
    uint64_t size;
    uint32_t type;
};


/*
解析BIOS E820中断获取的内存分布
生成Memory_Block [] 输出到 blocks(传入参数)的位置
同时寻找一个内存地址用于加载内核

输入：
entrys/entrys_num: BIOS E820中断获取的内存分布
kernel_size: 将要加载的内核大小(字节)
blocks: 一个内存地址，用来存放解析后的内存分布

输出：
blocks[blocks_num]: 存放着解析后的内存分布
kernel_load_address: 一个可以加载内核的地址
status:
-1/-2/-3 : 输入的entrys有误
-5: 可加载内核的地址在4GB以上(如果此函数在32位下编译)
-4：找不到可加载内核的地址
*/

struct
{
    ssize_t status;
    void *kernel_load_address;
    size_t blocks_num;
} handle_memory_map(E820_Entry*const entrys, size_t entrys_num, const size_t kernel_size, Memory_Block *const blocks)
{
    __typeof__(handle_memory_map(NULL, 0, 0, NULL)) ret;
    // check memory_map_entry_list
    for (size_t i=entrys_num; i!=0; )
    {
        --i;
        if (entrys[i].entry_size==24)
        {
            if ( (entrys[i]._extern&1) == 0  )
            {
                goto label1;
            }
        }
        else if ( entrys[i].entry_size != 20 )
        {
            ret.status=-3;
            return ret;
        }
        if ( entrys[i].size == 0 )
        {
            goto label1;
        }
        if ( entrys[i].type == 0 || entrys[i].type > 5 )
        {
            ret.status=-2;
            return ret;
        }
        continue;
label1:
        // remove entrys[i]
        --entrys_num;
        entrys[i]=entrys[entrys_num];
    }
    if ( entrys_num == 0 )
    {
        ret.status=-1;
        return ret;
    }

    size_t blocks_num=1;
    blocks[0].base=0;
    blocks[0].type=0;
    for ( size_t i=0; i<entrys_num; ++i )
    {
        for ( size_t i2=blocks_num; i2!=0; )
        {
            --i2;
            if ( blocks[i2].base == entrys[i].base )
            {
                break;
            }
            else if ( blocks[i2].base < entrys[i].base )
            {
                // insert in blocks[i2+1]
                memmove(&blocks[i2+2], &blocks[i2+1], sizeof(Memory_Block)*(blocks_num-i2-1));
                blocks[i2+1].base=entrys[i].base;
                blocks[i2+1].type=0;
                ++blocks_num;
                break;
            }
        }
        for ( size_t i2=blocks_num; i2!=0; )
        {
            --i2;
            if ( blocks[i2].base == entrys[i].base+entrys[i].size )
            {
                break;
            }
            else if ( blocks[i2].base < entrys[i].base+entrys[i].size )
            {
                // insert in blocks[i2+1]
                memmove(&blocks[i2+2], &blocks[i2+1], sizeof(Memory_Block)*(blocks_num-i2-1));
                blocks[i2+1].base=entrys[i].base+entrys[i].size;
                blocks[i2+1].type=0;
                ++blocks_num;
                break;
            }
        }
    }
    for ( size_t i=0; i<blocks_num; ++i )
    {
        for ( size_t i2=0; i2<entrys_num; ++i2 )
        {
            if ( entrys[i2].base<=blocks[i].base && blocks[i].base<entrys[i2].base+entrys[i2].size )
            {
                if ( entrys[i2].type > blocks[i].type )
                {
                    blocks[i].type=entrys[i2].type;
                }
            }
        }
    }
    blocks[blocks_num-1].size=(uint64_t)-1;
    for ( size_t i=0; i<blocks_num-1; ++i)
    {
        size_t num=0;
        while (1)
        {
            if ( blocks[i+num+1].type == blocks[i].type )
            {
                ++num;
            }
            else
            {
                break;
            }
        }
        // remove num of blocks in [i+1]
        if ( num != 0)
        {
            memmove(&blocks[i+1], &blocks[i+num+1], sizeof(Memory_Block)*(blocks_num-i-num-1));
            blocks_num-=num;
        }
        blocks[i].size=blocks[i+1].base-blocks[i].base;
    }
    ret.blocks_num=blocks_num;
    // 寻找一个加载内核的位置
    for ( size_t i=0; i<blocks_num; ++i )
    {
        if ( blocks[i].type == 1 )
        {
            uint64_t start=blocks[i].base;
            if ( start < (((uint64_t)16)<<20) )
            {
                start=(((uint64_t)16)<<20);
            }
            else
            {
                start=REMOVE_BITS_LOW(start-1, 21)+(((uint64_t)1)<<21);
            }
            if ( start+(uint64_t)kernel_size <= blocks[i+1].base )
            {
                // 在32位下编译时检查地址是否超过4GB
                if ( (start+kernel_size-1) < (uint64_t)(size_t)-1 )
                {
                    ret.status=0;
                    ret.kernel_load_address=(void *)(size_t)start;
                    return ret;
                }
                else
                {
                    ret.status=-5;
                    return ret;
                }
            }
        }
    }
    ret.status=-4;
    return ret;
}
