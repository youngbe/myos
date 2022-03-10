#include "libc.h"

// 由BIOS中断获得的Memeory_map信息
struct Memory_map_entry
{
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t _extern;
    uint8_t entry_size;
}__attribute__ ((packed));

static inline int my_compare(const void *, const void *);

// 处理由BIOS中断获取的memory_map，返回 %rsp
// 假设获取的memory_map中的各个条目间有互相重叠的部分：https://wiki.osdev.org/Memory_Map_(x86)
void *handle_memory_map( struct Memory_map_entry* entry_list, uint64_t entry_list_size, uint64_t kernel_sectors_num)
{
    // check memory_map_entry_list
    for (uint64_t i=entry_list_size; i!=0; )
    {
        --i;
        if (entry_list[i].entry_size==24)
        {
            if ( (entry_list[i]._extern&1) == 0  )
            {
                goto label1;
            }
        }
        else if ( entry_list[i].entry_size != 20 )
        {
            return NULL;
        }
        if ( entry_list[i].size != 0 )
        {
            if ( entry_list[i].type == 0 || entry_list[i].type > 5 )
            {
                return NULL;
            }
            continue;
        }
label1:
        --entry_list_size;
        entry_list[i]=entry_list[entry_list_size];
    }
    if ( entry_list_size == 0 )
    {
        return NULL;
    }

    uint64_t address_list[entry_list_size<<1];
    address_list[0]=0;
    uint64_t address_list_size=1;
    for ( uint64_t i=0; i<entry_list_size; ++i )
    {
        address_list_size=insert(address, address_list_size, entry_list[i].base);
        address_list_size=insert(address, address_list_size, entry_list[i].base+entry_list[i].size);
    }

    // 0 代表无效地址， 1 代表空闲属性， 2代表其他属性
    uint8_t type_list[address_list_size];
    memset(type_list, 0, address_list_size*sizeof(uint8_t));
    for ( uint64_t i=0; i<address_list_size; ++i )
    {
        for ( uint64_t i2=0; i2<entry_list_size; ++i2 )
        {
            if ( entry_list[i2].base<=address_list[i] && address_list[i]<entry_list[i2].base+entry_list[i2].size )
            {
                if ( entry_list[i2].type!=1 )
                {
                    type_list[i]=2;
                }
                else if ( type_list[i] == 0 )
                {
                    type_list[i]=1;
                }
            }
        }
    }

    for (uint64_t i=1; i<address_list_size; ++i)
    {
        while ( type_list[i-1] == type_list[i] )
        {
            --address_list_size;
            if ( i == address_list_size )
            {
                goto label_out;
            }
            memmove( &address_list[i], &address_list[i+1], (address_list_size-i)*sizeof(address_list[0]) );
            memmove( &type_list[i], &type_list[i+1], (address_list_size-i)*sizeof(type_list[0]) );
        }
    }
label_out:


}

inline int my_compare(const void *entry_a, const void *entry_b)
{
    if ( (const struct Memory_map_entry *)entry_a->base > (const struct Memory_map_entry *)entry_b->base )
    {
        return 1;
    }
    else if ( (const struct Memory_map_entry *)entry_a->base == (const struct Memory_map_entry *)entry_b->base )
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

