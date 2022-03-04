typedef unsigned long int uint64_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

#define NULL ((void *)0)


struct Memory_map_entry
{
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t _extern;
    uint8_t entry_size;
}__attribute__ ((packed));

void *handle_memory_map( struct Memory_map_entry* entry_list, uint64_t entry_list_size, uint64_t kernel_sectors_num)
{
    // check memory_map_entry_list
    if ( entry_list_size == 0 )
    {
        return NULL;
    }
    for (uint64_t i=entry_list_size-1; ; )
    {
        if ( entry_list[i].entry_size!=24 && entry_list[i].entry_size != 20 )
        {
            return NULL;
        }
        if ( i==0 )
        {
            break;
        }
        --i;
    }
    void *mem_start=(void *)(0x100000+kernel_sectors_num*512);
    // find a usable 2mb page
    for (uint64_t i=0; i<entry_list_size; ++i)
    {
        if (entry_list[i].entry_size==24)
        {
            if ( (entry_list[i]._extern&1) == 0 )
            {
                continue;
            }
        }
        if ( entry_list[i].size == 0 )
        {
            continue;
        }
        if ( entry_list[i].type != 1 )
        {
            continue;
        }
        if ( entry_list[i] )
    }
}
