#include <init.h>

extern void _start();
extern void __kernel_end();



void init(size_t block_nums, Memory_Block *)
{
    size_t const kernel_size=(size_t)__kernel_end-(size_t)_start;
    size_t const kernel_start=(size_t)_start;
    for ()
    {
    }
}
