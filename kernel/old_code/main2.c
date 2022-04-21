#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <bit.h>

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

typedef struct Memory_Block Memory_Block;
struct Memory_Block
{
    uint64_t base;
    uint64_t size;
    uint32_t type;
};

struct Page_Table;
struct Thread
{
    struct Page_Table *cr3;
};

extern int _start[];
extern int __kernel_end[];
void kernel_real_start();

struct __attribute__ ((packed)) Segment_Descriptor
{
    uint16_t limit0;
    uint16_t base0;
    uint8_t base1;
    uint8_t flag0;
    uint8_t limit1_flag1;
    uint8_t base2;
};

struct __attribute__ ((aligned(32),packed)) TSS64
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

struct __attribute__((packed)) GDT
{
    struct Segment_Descriptor null_d;
    struct Segment_Descriptor cs_d;
    struct Segment_Descriptor cs_user_d;
    struct Segment_Descriptor ds_user_d;
    struct Segment_Descriptor tssl_d;
    struct Segment_Descriptor tssh_d;
};


struct Page_Table
{
    uint64_t pte[512] __attribute__((packed));
    size_t num;
};


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
size_t cores_num;
struct TSS64 *tsss;
size_t free_threads_num;
struct Thread** free_threads;
size_t free_pages_num=0;
void **free_pages;
size_t firm_pages_num=0;
void **firm_pages;
size_t free_page_tables_num;
struct Page_Table **free_page_tables;
struct Page_Table *kernel_cr3;
// 内核栈

static inline void mark(Memory_Block*const blocks, size_t *const blocks_num, const size_t index, const uint64_t start, const uint64_t end, const uint32_t type);
static inline void* malloc_mark(size_t const size, size_t const p2align, const uint32_t type, Memory_Block *const blocks, size_t *const blocks_num);
// 将一个物理2m页映射到虚拟2m页
static inline bool map_page_2m(const uint64_t v_page, const uint64_t phy_page, struct Page_Table *cr3);
// 将一个物理4k页映射到虚拟4k页
static inline bool map_page_4k(const uint64_t v_page, const uint64_t phy_page, struct Page_Table *cr3);
static inline __attribute__((noreturn)) void error();
#define MAX_BLOCKS_NUM 512



void init(const Memory_Block *const blocks, const size_t blocks_num)
{
    if ( GET_BITS_LOW((uint64_t)_start, 5) != 0 )
    {
        // 内核加载位置aligned不足
        error();
    }


    // 获取cpu逻辑核心数量
    cores_num=8;

    // type:0 空闲块
    //1 1:1映射固化系统块
    //2 1:1映射非固化系统块
    //3 非1:1映射块
    Memory_Block usable_blocks[MAX_BLOCKS_NUM];
    size_t usable_blocks_num=0;
    size_t all_usable_size=0;


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
                error();
            }
            usable_blocks[usable_blocks_num].base=start;
            usable_blocks[usable_blocks_num].size=end-start;
            usable_blocks[usable_blocks_num].type=0;
            all_usable_size+=usable_blocks[usable_blocks_num].size;
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
    error();
label_next0:


    // TSS
    tsss=(struct TSS64*)malloc_mark(cores_num*sizeof(struct TSS64), 5, 1, usable_blocks, &usable_blocks_num);
    // 线程表
    free_threads_num=(all_usable_size/sizeof(struct Thread))>>6;
    if ( free_threads_num < 1024 )
    {
        error();
    }
    {
        struct Thread *free_threads_=(struct Thread *)malloc_mark( free_threads_num*sizeof(struct Thread), 4, 1, usable_blocks, &usable_blocks_num);
        free_threads=(struct Thread **)malloc_mark(free_threads_num*sizeof(struct Thread*), 3, 2, usable_blocks, &usable_blocks_num);
        for ( size_t i=0; i<free_threads_num; ++i )
        {
            free_threads[i]=&free_threads_[i];
        }
    }
    // 页数组(可用页/固化页)
    free_pages=(void**)malloc_mark((all_usable_size>>21)*sizeof(void*), 3, 2, usable_blocks, &usable_blocks_num);
    firm_pages=&free_pages[all_usable_size>>21];
    // 页表
    {
        size_t const aligned_page_table_size=REMOVE_BITS_LOW(sizeof(struct Page_Table)-1, 12) + ((size_t)1<<12);
        free_page_tables_num=(all_usable_size/(aligned_page_table_size+sizeof(struct Page_Table *)))>>5;
        free_page_tables=(struct Page_Table**)malloc_mark(free_page_tables_num*sizeof(struct Page_Table*), 3, 2, usable_blocks, &usable_blocks_num);
        uint8_t *temp=(uint8_t*)malloc_mark(free_page_tables_num*aligned_page_table_size, 12, 2, usable_blocks, &usable_blocks_num);
        for ( size_t i=0; i<free_page_tables_num; ++i )
        {
            *(struct Page_Table*)temp=(struct Page_Table){{0},0};
            free_page_tables[i]=(struct Page_Table *)temp;
            temp+=aligned_page_table_size;
        }
    }



    // 创建内核页表
    if (free_page_tables_num==0)
    {
        error();
    }
    kernel_cr3=free_page_tables[--free_page_tables_num];
    map_page_4k(0xb8000, 0xb8000, kernel_cr3);
    // 遍历一边固化页
    for ( size_t i=0; i<usable_blocks_num; ++i )
    {
        if ( usable_blocks[i].type == 1 )
        {
            uint64_t start=REMOVE_BITS_LOW(usable_blocks[i].base, 21);
            const uint64_t end=REMOVE_BITS_LOW(usable_blocks[i].base+usable_blocks[i].size-1, 21)+((uint64_t)1<<21);
            while ( start != end )
            {
                if (map_page_2m(start, start, kernel_cr3))
                {
                    firm_pages=&firm_pages[-1];
                    firm_pages[0]=(void*)start;
                    firm_pages_num++;
                }
                start+=(uint64_t)1<<21;
            }
        }
    }
    // 遍历一边普通页
    for ( size_t i=0; i<usable_blocks_num; ++i )
    {
        if ( usable_blocks[i].type == 2 )
        {
            uint64_t start=REMOVE_BITS_LOW(usable_blocks[i].base, 21);
            const uint64_t end=REMOVE_BITS_LOW(usable_blocks[i].base+usable_blocks[i].size-1, 21)+((uint64_t)1<<21);
            while ( start != end )
            {
                map_page_2m(start, start, kernel_cr3);
                start+=(uint64_t)1<<21;
            }
        }
    }
    // 内核栈
    {
        uint64_t const stack_page=(uint64_t)malloc_mark((uint64_t)1<<21, 21, 3, usable_blocks, &usable_blocks_num);
        map_page_2m(((uint64_t)1<<48)-((uint64_t)1<<21), stack_page, kernel_cr3);
    }
    // free_page
    for ( size_t i=0; i<usable_blocks_num; ++i )
    {
        if ( usable_blocks[i].type == 0 )
        {
            uint64_t start=REMOVE_BITS_LOW(usable_blocks[i].base-1, 21)+((uint64_t)1<<21);
            const uint64_t end=REMOVE_BITS_LOW(usable_blocks[i].base+usable_blocks[i].size, 21);
            while ( start<end )
            {
                free_pages[free_pages_num++]=(void*)start;
                start+=(uint64_t)1<<21;
            }
        }
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
    {
        __asm__ volatile (
                "movq   %[cr3], %%cr3\n\t"
                "movq   $0xFFFFFFFFFFF8, %%rsp\n\t"
                "jmpq   *%[start]"
                :
                :[start]"r"(kernel_real_start), [cr3]"r"(kernel_cr3)
                :"memory"
                );
        __builtin_unreachable();
    }

}


inline void* malloc_mark(size_t const size, size_t const p2align, const uint32_t type, Memory_Block *const blocks, size_t *const blocks_num)
{
    if ( size == 0 )
    {
        error();
    }
    if ( p2align < 3 || p2align >=64)
    {
        error();
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
    error();
}

inline void mark(Memory_Block*const blocks, size_t *const blocks_num, const size_t i, const uint64_t start, const uint64_t end, const uint32_t type)
{
    const uint64_t blocks_end=blocks[i].base+blocks[i].size;
    if ( start < blocks[i].base || end > blocks_end )
    {
        error();
    }
    if ( end <= start )
    {
        error();
    }
    if ( type == 0 )
    {
        error();
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
                error();
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
        error();
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
        error();
    }
    memmove(&blocks[i+3], &blocks[i+2], (*blocks_num-i-2)*sizeof(blocks[0]));
    blocks[i+2].base=end;
    blocks[i+2].size=blocks_end-end;
    blocks[i+2].type=0;
    ++*blocks_num;
}

inline bool map_page_2m(const uint64_t v_page, const uint64_t phy_page, struct Page_Table* pt)
{
    if ( v_page >= (((uint64_t)1)<<48) || phy_page >= (((uint64_t)1)<<48) )
    {
        error();
    }
    if ( GET_BITS_LOW(v_page, 21) != 0 || GET_BITS_LOW(phy_page, 21) != 0 )
    {
        error();
    }
    // cr3 -> 0级页表 -> 1级页表 -> 2级页表 -> 2mb物理页
    // 现在pt指向0级页表(pt == cr3)
    uint64_t i=v_page>>39;
    if ( pt->pte[i] == 0 )
    {
        if ( free_page_tables_num == 0 )
        {
            error();
        }
        struct Page_Table *new_page_table=free_page_tables[--free_page_tables_num];
        *new_page_table=(struct Page_Table){{0},0};
        pt->pte[i]=(uint64_t)new_page_table|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
        ++pt->num;
    }
    pt=(struct Page_Table*)REMOVE_BITS_LOW(pt->pte[i], 12);
    // 现在pt指向1级页表
    i=GET_BITS_LOW(v_page>>30, 9);
    if ( pt->pte[i] == 0 )
    {
        if ( free_page_tables_num == 0 )
        {
            error();
        }
        struct Page_Table *new_page_table=free_page_tables[--free_page_tables_num];
        *new_page_table=(struct Page_Table){{0}, 0};
        pt->pte[i]=(uint64_t)new_page_table|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
        ++pt->num;
    }
    pt=(struct Page_Table*)REMOVE_BITS_LOW(pt->pte[i], 12);
    // 现在pt指向2级页表
    i=GET_BITS_LOW(v_page>>21, 9);
    if ( pt->pte[i] != 0 )
    {
        return false;
    }
    pt->pte[i]=phy_page|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2)|((uint64_t)1<<7);
    ++pt->num;
    return true;
}

inline bool map_page_4k(const uint64_t v_page, const uint64_t phy_page, struct Page_Table* pt)
{
    if ( v_page >= (((uint64_t)1)<<48) || phy_page >= (((uint64_t)1)<<48) )
    {
        error();
    }
    if ( GET_BITS_LOW(v_page, 12) != 0 || GET_BITS_LOW(phy_page, 12) != 0 )
    {
        error();
    }
    // cr3 -> 0级页表 -> 1级页表 -> 2级页表 -> 3级页表 -> 4k物理页
    // 现在pt指向0级页表(pt == cr3)
    uint64_t i=v_page>>39;
    if ( pt->pte[i] == 0 )
    {
        if ( free_page_tables_num == 0 )
        {
            error();
        }
        struct Page_Table *new_page_table=free_page_tables[--free_page_tables_num];
        *new_page_table=(struct Page_Table){{0},0};
        pt->pte[i]=(uint64_t)new_page_table|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
        ++pt->num;
    }
    pt=(struct Page_Table*)REMOVE_BITS_LOW(pt->pte[i], 12);
    // 现在pt指向1级页表
    i=GET_BITS_LOW(v_page>>30, 9);
    if ( pt->pte[i] == 0 )
    {
        if ( free_page_tables_num == 0 )
        {
            error();
        }
        struct Page_Table *new_page_table=free_page_tables[--free_page_tables_num];
        *new_page_table=(struct Page_Table){{0}, 0};
        pt->pte[i]=(uint64_t)new_page_table|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
        ++pt->num;
    }
    pt=(struct Page_Table*)REMOVE_BITS_LOW(pt->pte[i], 12);
    // 现在pt指向2级页表
    i=GET_BITS_LOW(v_page>>21, 9);
    if ( pt->pte[i] == 0 )
    {
        if ( free_page_tables_num == 0 )
        {
            error();
        }
        struct Page_Table *new_page_table=free_page_tables[--free_page_tables_num];
        *new_page_table=(struct Page_Table){{0}, 0};
        pt->pte[i]=(uint64_t)new_page_table|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
        ++pt->num;
    }
    pt=(struct Page_Table*)REMOVE_BITS_LOW(pt->pte[i], 12);
    // 现在pt指向3级页表
    i=GET_BITS_LOW(v_page>>12, 9);
    if ( pt->pte[i] != 0 )
    {
        return false;
    }
    pt->pte[i]=phy_page|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
    ++pt->num;
    return true;
}

inline __attribute__((noreturn)) void error()
{
    struct __attribute__((packed)) Temp
    {
        uint16_t x[80*25];
    };
    *(struct Temp *)0xb8000=(struct Temp){ {[0 ... 80*25-1] = 0x0700} };
    /*
    for ( size_t i=0; i<80*25; ++i )
    {
        (*(uint16_t (*)[80*25])0xb8000)[i]=0x0700;
    }
    */
    (*(char (*)[80*25*2])0xb8000)[0]='E';
    (*(char (*)[80*25*2])0xb8000)[2]='r';
    (*(char (*)[80*25*2])0xb8000)[4]='r';
    (*(char (*)[80*25*2])0xb8000)[6]='o';
    (*(char (*)[80*25*2])0xb8000)[8]='r';
    (*(char (*)[80*25*2])0xb8000)[10]=' ';
    (*(char (*)[80*25*2])0xb8000)[12]='i';
    (*(char (*)[80*25*2])0xb8000)[14]='n';
    (*(char (*)[80*25*2])0xb8000)[16]=' ';
    (*(char (*)[80*25*2])0xb8000)[18]='i';
    (*(char (*)[80*25*2])0xb8000)[20]='n';
    (*(char (*)[80*25*2])0xb8000)[22]='i';
    (*(char (*)[80*25*2])0xb8000)[24]='t';
    (*(char (*)[80*25*2])0xb8000)[26]='!';
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
