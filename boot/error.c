#include <stdint.h>
#include <stddef.h>

__attribute__((noreturn))
void error()
{
    for ( size_t i=0; i<80*25; ++i )
    {
        (*(uint16_t (*)[80*25])0xb8000)[i]=0x0700;
    }
    (*(char (*)[80*25*2])0xb8000)[0]='E';
    (*(char (*)[80*25*2])0xb8000)[2]='r';
    (*(char (*)[80*25*2])0xb8000)[4]='r';
    (*(char (*)[80*25*2])0xb8000)[6]='o';
    (*(char (*)[80*25*2])0xb8000)[8]='r';
    (*(char (*)[80*25*2])0xb8000)[10]='!';
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
