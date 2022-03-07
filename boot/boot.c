typedef unsigned long int uint64_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef uint64_t size_t;

#define NULL ((void *)0)

// 由BIOS中断获得的Memeory_map信息
struct Memory_map_entry
{
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t _extern;
    uint8_t entry_size;
}__attribute__ ((packed));

static inline void * memcpy(void * destination, const void * source, size_t size);
static inline void * memmove(void * destination, const void * source, size_t size);
static inline int my_compare(const void *, const void *);
static inline void sort( void* base, size_t num, size_t width, int(*compare)(const void*,const void*) );

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
    struct Temp
    {
        uint64_t address;
        uint64_t index;
    } address_list[2*entry_list_size];
    sort(  );
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

inline void sort( void* base, size_t num, size_t width, int(*compare)(const void*,const void*) )
{
    for ( size_t i=1; i<num; ++i )
    {
        for ( size_t i2=0; i2<num-i; ++i2 )
        {
            if ( compare( (uint8_t *)base+width*i2, (uint8_t *)base+width*(i2+1) ) > 0 )
            {
                uint8_t temp[width];
                memcpy(temp, (uint8_t *)base+width*i2, width );
                memcpy((uint8_t *)base+width*i2, (uint8_t *)base+width*(i2+1) , width);
                memcpy((uint8_t *)base+width*(i2+1), temp, width);
            }
        }
    }
}

inline void * memcpy ( void * destination, const void * source, size_t num )
{
    {
        void *temp_d=destination;
        while (num!=0)
        {
            --num;
            *(uint8_t*)temp_d=*(const uint8_t*)source;
            temp_d=(uint8_t *)temp_d+1;
            source=(const uint8_t *)source+1;
        }
    }
    return destination;
}

inline void * memmove ( void * destination, const void * source, size_t num )
{
    if ( source > destination || destination >= (const void *)((const uint8_t *)source + num) )
    {
        void * temp_d=destination;
        while (num!=0)
        {
            --num;
            *(uint8_t*)temp_d=*(const uint8_t*)source;
            temp_d=(uint8_t *)temp_d+1;
            source=(const uint8_t *)source+1;
        }
    }
    else if ( source == destination )
    {
    }
    else
    {
        while (num!=0)
        {
            --num;
            ((uint8_t*)destination)[num]=((const uint8_t*)source)[num];
        }
    }
    return destination;
}
