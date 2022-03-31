#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Elf64_Rela Elf64_Rela;

struct __attribute__((packed)) Elf64_Rela
{
    uint64_t offset;
    uint64_t info;
    uint64_t append;
};

int main();

extern void (*__start_init_array[])();
extern void (*__stop_init_array[])();

extern void (*__start_fini_array[])();
extern void (*__stop_fini_array[])();

extern struct Elf64_Rela __start_rela_dyn[];
extern struct Elf64_Rela __stop_rela_dyn[];

__attribute__((section(".text.entry_point"), noreturn))
void _start()
{
    for (size_t i=0; &__start_rela_dyn[i] < __stop_rela_dyn; ++i)
    {
        if ( __start_rela_dyn[i].info != 8 )
        {
            while(1);
        }
        *(uint64_t volatile *)((size_t)_start+__start_rela_dyn[i].offset)+=__start_rela_dyn[i].append;
    }
    for ( size_t i=0; &__start_init_array[i]<__stop_init_array; ++i )
    {
        __start_init_array[i]();
    }
    main();
    for ( size_t i=0; &__start_fini_array[i]<__stop_fini_array; ++i )
    {
        __start_fini_array[i]();
    }
    {
        size_t* temp=(size_t *)0xb8000;
        while ( true )
        {
            *(size_t *)temp=0x0700070007000700;
            if ( temp >= (size_t *)(0xb8000+80*25*2-sizeof(size_t)) )
            {
                break;
            }
            ++temp;
        }
    }
    *(char *)0xb8000='K';
    *(char *)0xb8002='e';
    *(char *)0xb8004='r';
    *(char *)0xb8006='n';
    *(char *)0xb8008='e';
    *(char *)0xb800a='l';
    *(char *)0xb800c=' ';
    *(char *)0xb800e='e';
    *(char *)0xb8010='n';
    *(char *)0xb8002='d';
    *(char *)0xb8004='!';
    __asm__ volatile
        (
         "cli\n"
         "1:\n\t"
         "hlt\n\t"
         "jmp   1b"
         :
         :
         :"memory"
         );
    __builtin_unreachable();
label_error:
    size_t* temp=(size_t *)0xb8000;
    while ( true )
    {
        *(size_t *)temp=0x0700070007000700;
        if ( temp >= (size_t *)(0xb8000+80*25*2-sizeof(size_t)) )
        {
            break;
        }
        ++temp;
    }
    *(char *)0xb8000='E';
    *(char *)0xb8002='r';
    *(char *)0xb8004='r';
    *(char *)0xb8006='o';
    *(char *)0xb8008='r';
    *(char *)0xb800a='!';
    __asm__ volatile(
            "cli\n"
            "1:\n\t"
            "hlt\n\t"
            "jmp   1b"
            :
            :"m"(*(char (*)[80*160*2])0xb8000)
            :
            );
    __builtin_unreachable();
}
