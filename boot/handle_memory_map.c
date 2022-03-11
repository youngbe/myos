#include "libc.h"

struct Ret
{
    uint32_t kernel_start_address;
    uint32_t kernel_start_esp;
};

// 由BIOS中断获得的Memeory_map信息
struct Memory_map_entry
{
    uint64_t base;
    uint64_t size;
    uint32_t type;
    uint32_t _extern;
    uint8_t entry_size;
}__attribute__ ((packed));

struct Ret handle_memory_map( const struct Memory_map_entry* const entry_list, uint32_t entry_num, uint32_t kernel_sector_num, void *result_address )
{
    return (struct Ret){0x100000, 0x7c00};
}
