#include <stdint.h>
#include <stddef.h>

#include <system_table.h>

static struct Segment_Descriptor __attribute__((aligned (32))) gdt[6]={
    // NULL Descriptor
    {0, 0, 0, 0, 0, 0},
    // Code64 Descriptor
    {0, 0, 0, 0b10011010, 0b00100000, 0},
    // Code64 Descriptor for user
    {0, 0, 0, 0b11111010, 0b00100000, 0},
    // Data64 Descriptor for user
    {0, 0, 0, 0b11110010, 0, 0},
    // TSS Descriptor low
    {0x67, 0, 0, 0b10001001, 0 , 0},
    // TSS Descriptor high
    {0, 0, 0, 0 ,0 , 0}
};

static struct TSS64 __attribute__((aligned (32))) tss;

static uint8_t core0_stack[CORE_STACK_SIZE];

// 逻辑核心数量
size_t core_nums=1;
// 每个逻辑核心一个struct Segment_Descriptor [6]
struct Segment_Descriptor (*gdts)[6]=&gdt;
// 每个逻辑核心一个struct TSS64
struct TSS64 *tsss=&tss;
// 整个系统唯一一个idt
struct Interrupt_Gate_Descriptor64 __attribute__((aligned (32))) idt[256]=
{
    [0 ... 31]={0, __CS, 2, 0b10001110, 0, 0 ,0},
    [32]={0, __CS, 1, 0b10001110, 0, 0 ,0},
    [33 ... 255]={0, __CS, 2, 0b10001110, 0, 0 ,0}
};
// 每个逻辑核心一个core_stack，用于处理该核心的中断
uint8_t (*core_stacks)[CORE_STACK_SIZE]=&core0_stack;

void init_gdts()
{
    for ( size_t i=0; i<core_nums; ++i )
    {
        gdts[i][GDT_ENTRY_TSS].base0=(uint16_t)(size_t)&tsss[i];
        gdts[i][GDT_ENTRY_TSS].base1=(uint8_t)(((size_t)&tsss[i])>>16);
        gdts[i][GDT_ENTRY_TSS].base2=(uint8_t)(((size_t)&tsss[i])>>24);
        gdts[i][GDT_ENTRY_TSS+1].limit0=(uint16_t)(((size_t)&tsss[i])>>32);
        gdts[i][GDT_ENTRY_TSS+1].base0=(uint16_t)(((size_t)&tsss[i])>>48);
    }
}

void reload_gdtr(const size_t core_id)
{
    struct __attribute__((packed))
    {
        uint16_t limit;
        uint64_t base;
    } gdtr={sizeof(*gdts)-1, (uint64_t)(size_t)&gdts[core_id]};
    {
        uint64_t temp_pgdtr=(uint64_t)(size_t)&gdtr;
        uint64_t temp_ds=__DS;
        __asm__ volatile(
                "movq   %%rsp, %%rax\n\t"
                "pushq  %[ds]\n\t"
                "pushq  %%rax\n\t"
                "pushfq\n\t"
                "pushq  %[cs]\n\t"
                "leaq   .Lreinit_gdt%=(%%rip), %%rax\n\t"
                "pushq  %%rax\n\t"
                "lgdtq  (%[pgdtr])\n\t"
                "iretq\n"
                ".Lreinit_gdt%=:\n\t"
                "movq   %[ds], %%ds\n\t"
                "movq   %[ds], %%es\n\t"
                "movq   %[ds], %%fs\n\t"
                "movq   %[ds], %%gs"
                // 之所以将[pgdtr]放在写入一栏
                // 是为了防止编译器将 %[pgdtr] 放在 %rsp 上
                :[pgdtr]"+r"(temp_pgdtr), [ds]"+r"(temp_ds)
                :"m"(gdtr), "m"(gdts[core_id]), [cs]"i"(__CS)
                :"rax"
            );
    }
}

void init_tsss()
{
    for (size_t i=0; i<core_nums; ++i)
    {
        tsss[i]=(struct TSS64){0, 0, 0, 0, 0, (uint64_t)(size_t)core_stacks[i], (uint64_t)(size_t)core_stacks[i], 0, 0, 0, 0, 0, 0, 0, 0};
    }
}

void load_tr()
{
    __asm__ volatile(
            "ltrw   %[tss]"
            :
            :[tss]"rm"((uint16_t)__TSS), "m"(*(struct TSS64 (*)[])tsss)
            :);
}

__attribute__ ((interrupt))
void empty_isr(void *rsp)
{
}

__attribute__ ((interrupt)) void timmer_isr(void* rsp);

__attribute__ ((interrupt)) void keyboard_isr(void* rsp);

static inline void set_offset(size_t i, void* offset)
{
    idt[i].offset0=(uint16_t)(size_t)offset;
    idt[i].offset1=(uint16_t)(((size_t)offset)>>16);
    idt[i].offset2=(uint32_t)(((size_t)offset)>>32);
}

void init_idt()
{
    for (size_t i=0; i<256; ++i)
    {
        set_offset(i, (void *)empty_isr);
    }
    set_offset(32, (void *)timer_isr);
    set_offset(33, (void *)keyboard_isr);
}

void load_idtr()
{
    struct __attribute__((packed))
    {
        uint16_t limit;
        uint64_t base;
    } idtr={sizeof(idt)-1, (uint64_t)(size_t)&idt};
    __asm__ volatile (
            "lidtq  %[idtr]"
            :
            :[idtr]"m"(idtr), "m"(idt)
            :);
}
