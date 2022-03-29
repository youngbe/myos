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
    init_gdts();
    init_tsss();
    init_idt();

    reload_gdtr(get_coreid());
    load_tr();
    load_idtr();

    tclear();
    tputs("Hello world!");

    hang_up();
}
