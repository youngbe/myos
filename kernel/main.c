#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <system_table.h>
#include <terminal.h>
#include <hang_up.h>
#include <coreid.h>

__attribute__((noreturn))
void main()
{
    // 重新加载gdt
    init_gdt();
    reload_gdtr(get_coreid());
    // 加载idt
    init_idt_and_load_idtr();

    tclear();
    tputs("Hello world!");

    hang_up();
}
