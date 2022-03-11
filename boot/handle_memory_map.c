struct Ret
{
    uint32_t kernel_start_address;
    uint32_t kernel_start_esp;
};

struct Ret handle_memory_map( uint32_t memory_map_list_address, uint32_t memory_map_list_size )
{
    return (struct Ret){0x100000, 0x7c00};
}
