#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <terminal.h>
#include <hang_up.h>

__attribute__((noreturn))
void main()
{
    tclear();
    tputs("Hello world!");
    hang_up();
}
