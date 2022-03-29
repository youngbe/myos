#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <system_table.h>
#include <terminal.h>
#include <hang_up.h>

__attribute__((noreturn))
void main()
{
    // 重新加载gdt
    reinit_gdt();
    // 加载idt
    init_idt();

    tclear();
    tputs("Hello world!");

    hang_up();
}
