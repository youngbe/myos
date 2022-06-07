#define IN_KERNEL_INIT
#include "public.h"

#include <bit.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>


typedef struct Memory_Block Memory_Block;
struct __attribute__((packed)) Memory_Block
{
    uint64_t base;
    uint64_t size;
    uint32_t type;
};

struct __attribute__ ((packed)) Segment_Descriptor
{
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t flag0;
    uint8_t limit1_flag1;
    uint8_t base2;
};

struct __attribute__((packed)) GDT
{
    struct Segment_Descriptor null_d;
    struct Segment_Descriptor cs_d;
    struct Segment_Descriptor ds_d;
    struct Segment_Descriptor ds_user_d;
    struct Segment_Descriptor cs_user_d;
    struct Segment_Descriptor tssl_d;
    struct Segment_Descriptor tssh_d;
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


// 输入
extern "C" __attribute__((section(".text.entry_point"), noreturn)) void _start(const Memory_Block *const blocks, const size_t blocks_num);
extern "C" int __kernel_end[];
extern "C" void kernel_start();
extern "C" void empty_isr();
extern "C" void timer_isr();
//void keyboard_isr();
// Memory Map 信息 ( 16Mb以下)
// 64K可用栈 16Mb以下
// 覆盖所有可访问物理地址的直接映射表 16Mb以下
// 加载的内核 16MB以上，对齐4K，在可用内存页中

// 输出
size_t cores_num;

static struct GDT __attribute__((aligned(32))) gdt=
{
    // NULL Descriptor
    {0, 0, 0, 0, 0, 0},
    // Code64 Descriptor
    {0, 0, 0, 0b10011010, 0b00100000, 0},
    // Data64 Descriptor
    {0, 0, 0, 0b10010010, 0, 0},
    // Data64 Descriptor for user
    {0, 0, 0, 0b11110010, 0, 0},
    // Code64 Descriptor for user
    {0, 0, 0, 0b11111010, 0b00100000, 0},
    // TSS Descriptor low
    {0x67, 0, 0, 0b10001001, 0 , 0},
    // TSS Descriptor high
    {0, 0, 0, 0 ,0 , 0}
};
#define REPEAT_M(_, __, ___) {0, __CS, 0, 0b10001110, 0, 0, 0},
static struct __attribute__((packed))
{
    struct Interrupt_Gate_Descriptor64 igds[256];
} idt __attribute__((aligned (32)))={{
    BOOST_PP_REPEAT(256, REPEAT_M, _)
}};
#undef REPEAT_M
struct TSS64 *tsss;

uint64_t kernel_pt1s[64][512] __attribute__((aligned(4096)))={{0}};
#define REPEAT_M(_, i, __) (uint64_t)&kernel_pt1s[i]+1,
const uint64_t halt_pt0[512] __attribute__((aligned(4096)))=
{
    BOOST_PP_REPEAT(64, REPEAT_M, _)
};
#undef REPEAT_M
uint64_t (*page_tables)[512];
uint_fast16_t *pte_nums;
uint64_t (**free_page_tables)[512];
size_t free_page_tables_num;
// 内核页表

uint64_t *free_pages;
size_t free_pages_num;

struct __attribute__((aligned(16))){uint8_t padding[HALT_STACK_SIZE];}* halt_stacks;
Thread ** running_threads;


/* virtual memory map:
 * 0-4T 内核代码 page_tables (pte_nums free_page_tables free_pages TSS halt_stacks running_threads)
 * 4T-32T 内核堆
 * 32T-128T 用户代码
 * 128T-248T 用户堆
 * 248T-256T 用户栈
 * */


static inline void mark(Memory_Block*const blocks, size_t *const blocks_num, const size_t index, const uint64_t start, const uint64_t end, const uint32_t type);
static inline void* malloc_mark(size_t const size, size_t const p2align, const uint32_t type, Memory_Block *const blocks, size_t *const blocks_num);
static inline size_t calc_free_pages_num(const Memory_Block *const blocks, size_t const blocks_num);
static inline void init_free_pages(const Memory_Block *const blocks, size_t const blocks_num);
static inline uint64_t (*new_pt())[512];
static inline uint_fast16_t * get_pte_num(uint64_t (*)[512]);
static inline void map_page_2m(const uint64_t v_page, const uint64_t phy_page);
static inline void map_page_4k(const uint64_t v_page, const uint64_t phy_page);
#define MAX_BLOCKS_NUM 512



__attribute__((noreturn))
void init(const Memory_Block *const blocks, const size_t blocks_num)
{
    if ( GET_BITS_LOW((uint64_t)_start, 12) != 0 )
    {
        // 内核加载位置aligned不足
        kernel_abort("Error in init!");
    }

    // 获取内核逻辑核心数
    cores_num=8;
    
    const size_t core_id=get_coreid();


    // type:0 空闲块
    // 1: 内存已分配，且需要1:1映射到页表
    Memory_Block usable_blocks[MAX_BLOCKS_NUM];
    size_t usable_blocks_num=0;


    for ( size_t i=0; i<blocks_num; ++i )
    {
        if ( blocks[i].type != 1 )
        {
            continue;
        }
        uint64_t start=blocks[i].base;
        uint64_t end=blocks[i].base+blocks[i].size;
        // <16M or >256T
        if ( end < (((uint64_t)18)<<20) || start >= ((uint64_t)1<<48) )
        {
            continue;
        }
        if ( start <= (((uint64_t)16)<<20) )
        {
            start=(((uint64_t)16)<<20);
        }
        else
        {
            // 向上取整2M
            start=REMOVE_BITS_LOW(start-1, 21)+ (((uint64_t)2)<<20);
        }
        if ( end >= ((uint64_t)1<<48) )
        {
            end = (uint64_t)1<<48;
        }
        else
        {
            // 向下取整2M
            end=REMOVE_BITS_LOW(end, 21);
        }
        if ( end > start )
        {
            if ( usable_blocks_num == MAX_BLOCKS_NUM )
            {
                kernel_abort("Error in init!");
            }
            usable_blocks[usable_blocks_num].base=start;
            usable_blocks[usable_blocks_num].size=end-start;
            usable_blocks[usable_blocks_num].type=0;
            usable_blocks_num++;
        }
    }


    // 将一部分内存固化（0-4T）
    // 实际上只需要固化内核代码和page_tables
    // 但我们在这里还固化了pte_nums,free_page_tables,free_pages,tss,挂起栈，running_threads，这是为了方便起见

    // 可用块中删除内核代码，并加入到固化块
    for ( size_t i=0; i<usable_blocks_num; ++i )
    {
        if ( usable_blocks[i].base <= (uint64_t)_start && (uint64_t)__kernel_end <= usable_blocks[i].base+usable_blocks[i].size )
        {
            mark(usable_blocks, &usable_blocks_num, i, (uint64_t)_start, (uint64_t)__kernel_end, 1);
            goto label_next0;
        }
    }
    // 内核代码和不可用页交错
    kernel_abort("Error in init!");
label_next0:

    // 分配页表空间
    free_page_tables_num=calc_free_pages_num(usable_blocks, usable_blocks_num)<<3;
    if ( free_page_tables_num <= 256 )
    {
        kernel_abort("Error in init!");
    }
    page_tables=(uint64_t (*)[512])malloc_mark(
            sizeof(uint64_t [free_page_tables_num][512]), 12, 1, usable_blocks, &usable_blocks_num);
    pte_nums=(uint_fast16_t *)malloc_mark(
            sizeof(uint_fast16_t [free_page_tables_num]), 3, 1, usable_blocks, &usable_blocks_num);
    free_page_tables=(uint64_t (**)[512])malloc_mark(
            sizeof(uint64_t (*[free_page_tables_num])[512]), 3, 1, usable_blocks, &usable_blocks_num);

    // 分配TSS空间
    tsss=(struct TSS64*)malloc_mark(sizeof(struct TSS64 [cores_num]), 5, 1, usable_blocks, &usable_blocks_num);
    // 分配挂起栈空间
    halt_stacks=(__typeof__(halt_stacks))(uintptr_t)malloc_mark(cores_num*sizeof(*halt_stacks), 4, 1, usable_blocks, &usable_blocks_num);

    running_threads=(Thread **)malloc_mark(sizeof(Thread *[cores_num]), 3, 1, usable_blocks, &usable_blocks_num);


    // 分配空闲表空间
    {
        size_t const max_free_pages_num=calc_free_pages_num(usable_blocks, usable_blocks_num);
        free_pages=(uint64_t *)malloc_mark(sizeof(uint64_t [max_free_pages_num]), 3, 1, usable_blocks, &usable_blocks_num);
    }

    // 初始化空闲页栈
    init_free_pages(usable_blocks, usable_blocks_num);
    // 从这里开始可以使用kmalloc，但注意分配到的是虚拟地址


    // 初始化页表
    memset(page_tables, 0, sizeof(uint64_t [free_page_tables_num][512]));
    memset(pte_nums, 0, sizeof(uint_fast16_t [free_page_tables_num]));
    for ( size_t i=0; i<free_page_tables_num; ++i )
    {
        free_page_tables[i]=&page_tables[free_page_tables_num-i-1];
    }
    map_page_4k(0xb8000, 0xb8000);
    // 遍历一边固化页
    for ( size_t i=0; i<usable_blocks_num; ++i )
    {
        if ( usable_blocks[i].type == 1 )
        {
            uint64_t start=REMOVE_BITS_LOW(usable_blocks[i].base, 21);
            const uint64_t end=REMOVE_BITS_LOW(usable_blocks[i].base+usable_blocks[i].size-1, 21)+((uint64_t)1<<21);
            while ( start != end )
            {
                // 大于4T
                if ( start >= ((uint64_t)1<<42)  )
                {
                    kernel_abort("Error in init!");
                }
                map_page_2m(start, start);
                start+=(uint64_t)1<<21;
            }
        }
    }


    // 初始化TSS
    for ( size_t i=0; i<cores_num; ++i )
    {
        tsss[i]=(struct TSS64){
            0,
                (uint64_t)&halt_stacks[i+1], 0, 0,
                0,
                0, 0, 0, 0, 0, 0, 0,
                0, 0, 0
        };
    }

    // 初始化Threads
    for ( size_t i=0; i<cores_num; ++i )
    {
        running_threads[i]=NULL;
    }

    // 加载gdtr
    {
        struct __attribute__((packed))
        {
            uint16_t limit;
            uint64_t base;
        } gdtr={sizeof(gdt)-1, (uint64_t)&gdt};
        uint64_t temp_ds=__DS;
        __asm__ volatile(
                "lgdtq  %[gdtr]\n\t"
                "movq   %[ds], %%ss\n\t"
                "movq   %[ds], %%ds\n\t"
                "movq   %[ds], %%es\n\t"
                "movq   %[ds], %%fs\n\t"
                "movq   %[ds], %%gs\n\t"
                "pushq  %[cs]\n\t"
                "leaq   .Lreinit_gdt%=(%%rip), %[ds]\n\t"
                "pushq  %[ds]\n\t"
                "lretq\n"
                ".Lreinit_gdt%=:"
                // 之所以将[pgdtr]放在写入一栏
                // 是为了防止编译器将 %[pgdtr] 放在 %rsp 上
                :[ds]"+r"(temp_ds)
                :[gdtr]"m"(gdtr), "m"(gdt), [cs]"i"(__CS)
                :
                );
    }
    // init syscall sysret
    {
        // IA32_STAR
        wrmsr(0xC0000081, ((uint64_t)__CS<<32)|((uint64_t)(__DS_USER-8)<<48) );
        // IA32_LSTAR
        //wrmsr(0xC0000082);
        // FMASK
        wrmsr(0xC0000084, (uint32_t)~(1<<9));
    }
    // 初始化idt, 加载idtr
    {
        
        for (size_t i=0; i<256; ++i)
        {
            idt.igds[i].offset0=(uint16_t)(uintptr_t)empty_isr;
            idt.igds[i].offset1=(uint16_t)(((uintptr_t)empty_isr)>>16);
            idt.igds[i].offset2=(uint32_t)(((uintptr_t)empty_isr)>>32);
        }
        idt.igds[32].offset0=(uint16_t)(uintptr_t)timer_isr;
        idt.igds[32].offset1=(uint16_t)(((uintptr_t)timer_isr)>>16);
        idt.igds[32].offset2=(uint32_t)(((uintptr_t)timer_isr)>>32);
        //idt.igds[33].offset0=(uint16_t)(uintptr_t)keyboard_isr;
        //idt.igds[33].offset1=(uint16_t)(((uintptr_t)keyboard_isr)>>16);
        //idt.igds[33].offset2=(uint32_t)(((uintptr_t)keyboard_isr)>>32);
        struct __attribute__((packed))
        {
            uint16_t limit;
            uint64_t base;
        } idtr={sizeof(idt)-1, (uint64_t)&idt};
        __asm__ volatile (
                "lidtq  %[idtr]"
                :
                :[idtr]"m"(idtr), "m"(idt)
                :);
    }
    // 加载tr
    {
        gdt.tssl_d.base0=(uint16_t)(uintptr_t)&tsss[core_id];
        gdt.tssl_d.base1=(uint8_t)(((uintptr_t)&tsss[core_id])>>16);
        gdt.tssl_d.base2=(uint8_t)(((uintptr_t)&tsss[core_id])>>24);
        gdt.tssh_d.limit0=(uint16_t)(((uintptr_t)&tsss[core_id])>>32);
        gdt.tssh_d.base0=(uint16_t)(((uintptr_t)&tsss[core_id])>>48);
        __asm__ volatile(
                "ltrw   %[tss]"
                :
                :"m"(gdt), [tss]"r"((uint16_t)__TSS)
                :);
    }
    // 切换cr3，切换栈，跳转执行
    __asm__ volatile (
            "movq   %[cr3], %%cr3\n\t"
            "movq   %[rsp], %%rsp\n\t"
            "sti\n\t"
            "jmp    kernel_start"
            :
            // 让 gcc 生成kernel_start函数
            :"X"(kernel_start), [cr3]"r"((uint64_t)&halt_pt0), [rsp]"r"((uint64_t)&halt_stacks[core_id+1]-8)
            :"memory"
            );
    __builtin_unreachable();
}


inline void* malloc_mark(size_t const size, size_t const p2align, const uint32_t type, Memory_Block *const blocks, size_t *const blocks_num)
{
    if ( size == 0 )
    {
        kernel_abort("Error in init!");
    }
    if ( p2align < 3 || p2align >=64)
    {
        kernel_abort("Error in init!");
    }
    for ( size_t i=0; i<*blocks_num; ++i )
    {
        if ( blocks[i].type != 0 )
        {
            continue;
        }
        const uint64_t aligned_base=REMOVE_BITS_LOW(blocks[i].base-1, p2align)+(((uint64_t)1)<<p2align);
        const uint64_t end=blocks[i].base+blocks[i].size;
        if ( end > aligned_base && end-aligned_base >= size )
        {
            mark(blocks, blocks_num, i, aligned_base, aligned_base+size, type);
            return (void*)aligned_base;
        }
    }
    kernel_abort("Error in init!");
}

inline void mark(Memory_Block*const blocks, size_t *const blocks_num, const size_t i, const uint64_t start, const uint64_t end, const uint32_t type)
{
    const uint64_t blocks_end=blocks[i].base+blocks[i].size;
    if ( start < blocks[i].base || end > blocks_end || end <= start || type == 0 )
    {
        kernel_abort("Error in init!");
    }
    if ( start == blocks[i].base )
    {
        if ( end == blocks_end )
        {
            blocks[i].type=type;
        }
        else
        {
            if ( *blocks_num == MAX_BLOCKS_NUM )
            {
                kernel_abort("Error in init!");
            }
            memmove(&blocks[i+2], &blocks[i+1], (*blocks_num-i-1)*sizeof(blocks[0]));
            ++*blocks_num;
            blocks[i+1].base=end;
            blocks[i+1].size=blocks_end-end;
            blocks[i+1].type=0;
            blocks[i].size=end-blocks[i].base;
            blocks[i].type=type;
        }
        return;
    }
    if ( *blocks_num == MAX_BLOCKS_NUM )
    {
        kernel_abort("Error in init!");
    }
    memmove(&blocks[i+2], &blocks[i+1], (*blocks_num-i-1)*sizeof(blocks[0]));
    ++*blocks_num;
    blocks[i].size=start-blocks[i].base;
    blocks[i+1].base=start;
    blocks[i+1].size=end-start;
    blocks[i+1].type=type;
    if ( end == blocks_end )
    {
        return;
    }
    if ( *blocks_num == MAX_BLOCKS_NUM )
    {
        kernel_abort("Error in init!");
    }
    memmove(&blocks[i+3], &blocks[i+2], (*blocks_num-i-2)*sizeof(blocks[0]));
    ++*blocks_num;
    blocks[i+2].base=end;
    blocks[i+2].size=blocks_end-end;
    blocks[i+2].type=0;
}

inline void map_page_2m(const uint64_t v_page, const uint64_t phy_page)
{
    if ( v_page >= (((uint64_t)1)<<48) || phy_page >= (((uint64_t)1)<<48) )
    {
        kernel_abort("Error in init!");
    }
    if ( GET_BITS_LOW(v_page, 21) != 0 || GET_BITS_LOW(phy_page, 21) != 0 )
    {
        kernel_abort("Error in init!");
    }
    // cr3(pt0) -> 0级页表.pt1 -> 1级页表.pt2 -> 2级页表 -> 2mb物理页
    uint64_t i=v_page>>39;
    // i一定小于64
    uint64_t (*const pt1)[512]=&kernel_pt1s[i];
    i=GET_BITS_LOW(v_page>>30, 9);
    uint64_t (*pt2)[512];
    if ( (*pt1)[i] == 0 )
    {
        pt2=new_pt();
        (*pt1)[i]=(uint64_t)pt2|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
    }
    else
    {
        pt2=(uint64_t (*)[512])REMOVE_BITS_LOW((*pt1)[i], 12);
    }
    i=GET_BITS_LOW(v_page>>21, 9);
    if ( (*pt2)[i] != 0 )
    {
        return;
    }
    (*pt2)[i]=phy_page|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2)|((uint64_t)1<<7);
    ++*get_pte_num(pt2);
}

inline void map_page_4k(const uint64_t v_page, const uint64_t phy_page)
{
    if ( v_page >= (((uint64_t)1)<<48) || phy_page >= (((uint64_t)1)<<48) )
    {
        kernel_abort("Error in init!");
    }
    if ( GET_BITS_LOW(v_page, 12) != 0 || GET_BITS_LOW(phy_page, 12) != 0 )
    {
        kernel_abort("Error in init!");
    }
    // cr3 -> 0级页表 -> 1级页表 -> 2级页表 -> 3级页表 -> 4k物理页
    uint64_t i=v_page>>39;
    // i 一定小于64
    uint64_t (*const pt1)[512]=&kernel_pt1s[i];
    i=GET_BITS_LOW(v_page>>30, 9);
    uint64_t (*pt2)[512];
    if ( (*pt1)[i] == 0 )
    {
        pt2=new_pt();
        (*pt1)[i]=(uint64_t)pt2|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
    }
    else
    {
        pt2=(uint64_t (*)[512])REMOVE_BITS_LOW((*pt1)[i], 12);
    }
    i=GET_BITS_LOW(v_page>>21, 9);
    uint64_t (*pt3)[512];
    if ( (*pt2)[i] == 0 )
    {
        pt3=new_pt();
        (*pt2)[i]=(uint64_t)pt3|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
        ++*get_pte_num(pt2);
    }
    else
    {
        pt3=(uint64_t (*)[512])REMOVE_BITS_LOW((*pt2)[i], 12);
    }
    i=GET_BITS_LOW(v_page>>12, 9);
    if ( (*pt3)[i] != 0 )
    {
        return;
    }
    (*pt3)[i]=phy_page|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
    ++*get_pte_num(pt3);
}

inline uint64_t (*new_pt())[512]
{
    if ( free_page_tables_num == 0 )
    {
        kernel_abort("Error in init!");
    }
    return free_page_tables[--free_page_tables_num];
}

inline uint_fast16_t * get_pte_num(uint64_t (*pt)[512])
{
    return &pte_nums[pt-page_tables];
}

inline size_t calc_free_pages_num(const Memory_Block *const blocks, size_t const blocks_num)
{
    size_t ret=0;
    for ( size_t i=0; i<blocks_num; ++i )
    {
        if (blocks[i].type != 0)
        {
            continue;
        }
        uint64_t const start=REMOVE_BITS_LOW(blocks[i].base-1, 21) + ((uint64_t)1<<21);
        uint64_t const end=REMOVE_BITS_LOW(blocks[i].base+blocks[i].size, 21);
        if ( end > start )
        {
            ret+=(end-start)>>21;
        }
    }
    return ret;
}
inline void init_free_pages(const Memory_Block *const blocks, size_t const blocks_num)
{
    free_pages_num=0;
    for ( size_t i=blocks_num; i!=0; )
    {
        --i;
        if (blocks[i].type != 0)
        {
            continue;
        }
        uint64_t const start=REMOVE_BITS_LOW(blocks[i].base-1, 21) + ((uint64_t)1<<21);
        uint64_t end=REMOVE_BITS_LOW(blocks[i].base+blocks[i].size, 21) - ((uint64_t)1<<21);
        while ( end >= start )
        {
            free_pages[free_pages_num++]=end;
            end-=(uint64_t)1<<21;
        }
    }
}
#undef IN_KERNEL_INIT
