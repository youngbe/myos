#pragma once
#include <stddef.h>

#define __CS        (0<<3)
#define __CS_USER   ((1<<3)|0b11)
#define __DS        (2<<3)
#define __DS_USER   ((3<<3)|0b11)
#define __TSS       (4<<3)

// 内核栈(每个逻辑核心一个)的大小，必须对齐16字节
#define CORE_STACK_SIZE 8192

static inline size_t get_coreid()
{
    return 0;
}

    __attribute__((noreturn))
static inline void kernel_abort(const char * str)
{
    struct __attribute__((packed)) Temp
    {
        uint16_t x[80*25];
    };
    *(struct Temp *)0xb8000=(struct Temp){ {[0 ... 80*25-1] = 0x0700} };
    char *v=(char *)0xb8000;
    while ( *str != '\0' )
    {
        *v=*str;
        ++str;
        v+=2;
    }
    __asm__ volatile(
            "cli\n"
            "1:\n\t"
            "hlt\n\t"
            "jmp   1b"
            :
            :"m"(*(char (*)[80*25*2])0xb8000)
            :
            );
    __builtin_unreachable();
}
