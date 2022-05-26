#include <tools.h>

#include <stdint.h>


extern "C"
{
    __attribute__((noreturn)) static inline void kernel_abort(const char * str)
    {
        struct __attribute__((packed)) Temp
        {
            uint16_t x[80*25];
        };
        typedef struct Temp Temp;
        #define REPEAT_MACRO(z, n, d) 0x0700,
        *(Temp *)0xb8000=(Temp){{BOOST_PP_REPEAT(2000, REPEAT_MACRO, _)}};
        #undef REPEAT_MACRO
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
}
