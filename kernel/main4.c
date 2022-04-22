#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <bit.h>

#include <kernel.h>

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
    struct Segment_Descriptor cs_user_d;
    struct Segment_Descriptor ds_user_d;
    struct Segment_Descriptor tssl_d;
    struct Segment_Descriptor tssh_d;
};


// 输入
__attribute__((section(".text.entry_point"), noreturn)) void _start(const Memory_Block *const blocks, const size_t blocks_num);
extern int __kernel_end[];
void kernel_real_start();
// Memory Map 信息 ( 16Mb以下)
// 64K可用栈 16Mb以下
// 覆盖所有可访问物理地址的直接映射表 16Mb以下
// 加载的内核 16MB以上，对齐4K，在可用内存页中

// 输出
static struct GDT __attribute__((aligned(32))) gdt=
{
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
size_t free_pages_num=0;
uint64_t *free_pages;
uint64_t (*page_tables)[512];
uint64_t (**free_page_tables)[512];
size_t free_page_tables_num;
uint_fast16_t *pte_nums;
// 内核栈

static inline void mark(Memory_Block*const blocks, size_t *const blocks_num, const size_t index, const uint64_t start, const uint64_t end, const uint32_t type);
static inline void* malloc_mark(size_t const size, size_t const p2align, const uint32_t type, Memory_Block *const blocks, size_t *const blocks_num);
static inline size_t calc_free_pages_num(const Memory_Block *const blocks, size_t const blocks_num);
static inline void init_free_pages(const Memory_Block *const blocks, size_t const blocks_num);
static inline uint64_t (*new_pt())[512];
static inline uint_fast16_t * get_pte_num(uint64_t (*)[512]);
static inline void map_page_2m(const uint64_t v_page, const uint64_t phy_page, uint64_t (*const cr3)[512]);
static inline void map_page_4k(const uint64_t v_page, const uint64_t phy_page, uint64_t (*const cr3)[512]);
#define MAX_BLOCKS_NUM 512

/* virtual memory map:
 * 0-4T kernel text data rodata page_tables free_pages
 * 4T-32T kernel heap stack
 * 30T-32T kernel stack
 * 32T-64T user data text rodata bss
 * 64T-248T user malloc
 * 248T-256T user stack
 * */


__attribute__((noreturn))
void main(const Memory_Block *const blocks, const size_t blocks_num)
{
    if ( GET_BITS_LOW((uint64_t)_start, 12) != 0 )
    {
        // 内核加载位置aligned不足
        kernel_abort("Error in init!");
    }


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

    {
        compiletime_assert(sizeof(*page_tables)==((size_t)1<<12), "Page Table size error!");
        free_page_tables_num=calc_free_pages_num(usable_blocks, usable_blocks_num)<<2;
        if ( free_page_tables_num <= 128 )
        {
            kernel_abort("Error in init!");
        }

        page_tables=malloc_mark(free_page_tables_num*sizeof(*page_tables), 12, 1, usable_blocks, &usable_blocks_num);
        memset(page_tables, 0, free_page_tables_num*sizeof(*page_tables));

        free_page_tables=malloc_mark(sizeof(void*)*free_page_tables_num, 3, 1, usable_blocks, &usable_blocks_num);
        for ( size_t i=0; i<free_page_tables_num; ++i )
        {
            free_page_tables[i]=&page_tables[free_page_tables_num-i-1];
        }

        pte_nums=malloc_mark(sizeof(*pte_nums)*free_page_tables_num, 3, 1, usable_blocks, &usable_blocks_num);
        memset(pte_nums, 0, sizeof(*pte_nums)*free_page_tables_num);
    }

    // free page
    {
        size_t const max_free_pages_num=calc_free_pages_num(usable_blocks, usable_blocks_num);
        free_pages=(uint64_t *)malloc_mark(max_free_pages_num*sizeof(uint64_t), 3, 1, usable_blocks, &usable_blocks_num);
        init_free_pages(usable_blocks, usable_blocks_num);
    }



    // init page
    for ( size_t i=0; i<64; ++i )
    {
        page_tables[64][i]=(uint64_t)&page_tables[i]|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
        pte_nums[i]=1;
    }
    pte_nums[64]=64;
    map_page_4k(0xb8000, 0xb8000, &page_tables[64]);
    // 遍历一边固化页(free_pages 以及 内核代码)
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
                map_page_2m(start, start, &page_tables[64]);
                start+=(uint64_t)1<<21;
            }
        }
    }
    // 内核栈
    {
        if ( free_pages_num == 0 )
        {
            kernel_abort("Error in init!");
        }
        uint64_t const stack_page=free_pages[--free_pages_num];
        // map to 32T
        map_page_2m(((uint64_t)1<<45)-((uint64_t)1<<21), stack_page, &page_tables[64]);
    }


    // gdt idt tss初始化
    {
        struct __attribute__((packed))
        {
            uint16_t limit;
            uint64_t base;
        } gdtr={sizeof(gdt)-1, (uint64_t)&gdt};
        uint64_t temp_pgdtr=(uint64_t)&gdtr;
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
                :"m"(gdtr), "m"(gdt), [cs]"i"(__CS)
                :"rax"
                );
    }
    // 切换cr3，切换栈，跳转执行
    __asm__ volatile (
            "movq   %[cr3], %%cr3\n\t"
            "movq   $0x1FFFFFFFFFF8, %%rsp\n\t"
            "jmp    kernel_real_start"
            :
            // 让 gcc 生成kernel_real_start函数
            :"X"(kernel_real_start), [cr3]"r"((uint64_t)&page_tables[64])
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
    if ( start < blocks[i].base || end > blocks_end )
    {
        kernel_abort("Error in init!");
    }
    if ( end <= start )
    {
        kernel_abort("Error in init!");
    }
    if ( type == 0 )
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
            blocks[i+1].base=end;
            blocks[i+1].size=blocks_end-end;
            blocks[i+1].type=0;
            blocks[i].size=end-blocks[i].base;
            blocks[i].type=type;
            ++*blocks_num;
        }
        return;
    }
    if ( *blocks_num == MAX_BLOCKS_NUM )
    {
        kernel_abort("Error in init!");
    }
    memmove(&blocks[i+2], &blocks[i+1], (*blocks_num-i-1)*sizeof(blocks[0]));
    blocks[i].size=start-blocks[i].base;
    blocks[i+1].base=start;
    blocks[i+1].size=end-start;
    blocks[i+1].type=type;
    ++*blocks_num;
    if ( end == blocks_end )
    {
        return;
    }
    if ( *blocks_num == MAX_BLOCKS_NUM )
    {
        kernel_abort("Error in init!");
    }
    memmove(&blocks[i+3], &blocks[i+2], (*blocks_num-i-2)*sizeof(blocks[0]));
    blocks[i+2].base=end;
    blocks[i+2].size=blocks_end-end;
    blocks[i+2].type=0;
    ++*blocks_num;
}

inline void map_page_2m(const uint64_t v_page, const uint64_t phy_page, uint64_t (*const cr3)[512])
{
    if ( v_page >= (((uint64_t)1)<<48) || phy_page >= (((uint64_t)1)<<48) )
    {
        kernel_abort("Error in init!");
    }
    if ( GET_BITS_LOW(v_page, 21) != 0 || GET_BITS_LOW(phy_page, 21) != 0 )
    {
        kernel_abort("Error in init!");
    }
    // cr3 -> 0级页表 -> 1级页表 -> 2级页表 -> 2mb物理页
    uint64_t i=v_page>>39;
    uint64_t (*pt1)[512];
    if ( (*cr3)[i] == 0 )
    {
        pt1=new_pt();
        (*cr3)[i]=(uint64_t)pt1|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
        ++*get_pte_num(cr3);
    }
    else
    {
        pt1=(uint64_t (*)[512])REMOVE_BITS_LOW((*cr3)[i], 12);
    }
    i=GET_BITS_LOW(v_page>>30, 9);
    uint64_t (*pt2)[512];
    if ( (*pt1)[i] == 0 )
    {
        pt2=new_pt();
        (*pt1)[i]=(uint64_t)pt2|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
        ++*get_pte_num(pt1);
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

inline void map_page_4k(const uint64_t v_page, const uint64_t phy_page, uint64_t (*const cr3)[512])
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
    uint64_t (*pt1)[512];
    if ( (*cr3)[i] == 0 )
    {
        pt1=new_pt();
        (*cr3)[i]=(uint64_t)pt1|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
        ++*get_pte_num(cr3);
    }
    else
    {
        pt1=(uint64_t (*)[512])REMOVE_BITS_LOW((*cr3)[i], 12);
    }
    i=GET_BITS_LOW(v_page>>30, 9);
    uint64_t (*pt2)[512];
    if ( (*pt1)[i] == 0 )
    {
        pt2=new_pt();
        (*pt1)[i]=(uint64_t)pt2|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
        ++*get_pte_num(pt1);
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
    return;
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
    for ( size_t i=0; i<blocks_num; ++i )
    {
        if (blocks[i].type != 0)
        {
            continue;
        }
        uint64_t start=REMOVE_BITS_LOW(blocks[i].base-1, 21) + ((uint64_t)1<<21);
        uint64_t const end=REMOVE_BITS_LOW(blocks[i].base+blocks[i].size, 21);
        while ( end > start )
        {
            free_pages[free_pages_num++]=start;
            start+=(uint64_t)1<<21;
        }
    }
}
