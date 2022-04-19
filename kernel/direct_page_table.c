#include <stdio.h>

int main()
{
    puts("  .section    direct_page_table");
    puts("  .align 4096");
    puts("  .globl  __direct_page_table_start");
    puts("__direct_page_table_start:");
    puts("  .globl  direct_cr3");
    puts("direct_cr3:");
    for ( size_t i=0; i<512; ++i )
    {
        printf("    .quad   .Lpts+0x%lx+0b11\n", i*4096);
    }
    puts(".Lpts:");
    for ( size_t i=0; i<512*512; ++i )
    {
        printf("    .quad   0x%lx+0b10000011\n", i*((size_t)1<<30));
    }
    puts("  .globl  __direct_page_table_end");
    puts("__direct_page_table_end:");
    return 0;
}
