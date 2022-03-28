#pragma once

__attribute__((noreturn))
static inline void hang_up()
{
    __asm__ volatile
        (
         ".Lhang_up%=:\n\t"
         "hlt\n\t"
         "jmp   .Lhang_up%="
         :
         :
         // 让 gcc 寄存器中的值写入内存后再挂起
         :"memory"
         );
    __builtin_unreachable();
}
