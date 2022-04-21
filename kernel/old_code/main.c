#include <init.h>
#include <sched.h>

__attribute__((noreturn))
void main(int block_nums, Memory_Block* blocks)
{
    *(char *)0xb8000='x';
    //init(block_nums, blocks);
    hang_up();
}
