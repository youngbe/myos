#pragma once

#define GDT_ENTRY_NULL      0
#define GDT_ENTRY_CS        1
#define GDT_ENTRY_CS_USER   2
#define GDT_ENTRY_DS_USER   3
#define GDT_ENTRY_TSS       4

#define __CS        (GDT_ENTRY_CS<<3)
#define __CS_USER   ((GDT_ENTRY_CS_USER<<3)|0b11)
#define __DS        (GDT_ENTRY_NULL<<3)
#define __DS_USER   ((GDT_ENTRY_DS_USER<<3)|0b11)
#define __TSS       (GDT_ENTRY_TSS<<3)

struct __attribute__ ((packed)) Segment_Descriptor
{
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t flag0;
    uint8_t limit1_flag1;
    uint8_t base2;
};

struct __attribute__ ((packed)) TSS64
{
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t io_map_base_address;
};

struct __attribute__ ((packed)) Interrupt_Gate_Descriptor64
{
    uint16_t offset0;
    uint16_t segment_selector;
    uint8_t  ist;
    uint8_t  flag;
    uint16_t offset1;
    uint32_t offset2;
    uint32_t reserve;
};

extern struct Interrupt_Gate_Descriptor64 idt[256];
extern struct TSS64 tss;
extern struct Segment_Descriptor gdt[6];

void reinit_gdt();
void init_idt();

__attribute__ ((interrupt)) void empty_isr(void *rsp);
__attribute__ ((interrupt)) void timer_isr(void* rsp);
__attribute__ ((interrupt)) void keyboard_isr(void* rsp);
