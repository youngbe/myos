#pragma once
#include <terminal.h>
#include <hang_up.h>

__attribute__ ((noreturn))
static inline void error()
{
    tclear();
    tputs("error!");
    hang_up();
}
