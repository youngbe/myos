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

void main(void *, void *);

extern struct Elf64_Rela __start_rela_dyn[];
extern struct Elf64_Rela __stop_rela_dyn[];

__attribute__((section(".text.entry_point"), noreturn))
void _start(void *arg1, void *arg2)
{
    for (size_t i=0; &__start_rela_dyn[i] < __stop_rela_dyn; ++i)
    {
        if ( __start_rela_dyn[i].info != 8 )
        {
            goto label_error;
        }
        *(uint64_t volatile *)((size_t)_start+__start_rela_dyn[i].offset)=(uint64_t)((size_t)_start+__start_rela_dyn[i].append);
    }
    main(arg1, arg2);
    for ( size_t i=0; i<80*25; ++i )
    {
        (*(uint16_t (*)[80*25])0xb8000)[i]=0x0700;
    }
    (*(char (*)[80*25*2])0xb8000)[0]='K';
    (*(char (*)[80*25*2])0xb8000)[2]='e';
    (*(char (*)[80*25*2])0xb8000)[4]='r';
    (*(char (*)[80*25*2])0xb8000)[6]='n';
    (*(char (*)[80*25*2])0xb8000)[8]='e';
    (*(char (*)[80*25*2])0xb8000)[10]='l';
    (*(char (*)[80*25*2])0xb8000)[12]=' ';
    (*(char (*)[80*25*2])0xb8000)[14]='e';
    (*(char (*)[80*25*2])0xb8000)[16]='x';
    (*(char (*)[80*25*2])0xb8000)[18]='i';
    (*(char (*)[80*25*2])0xb8000)[20]='t';
    (*(char (*)[80*25*2])0xb8000)[22]='e';
    (*(char (*)[80*25*2])0xb8000)[24]='d';
    (*(char (*)[80*25*2])0xb8000)[26]='!';
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
