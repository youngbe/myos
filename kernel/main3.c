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
size_t free_ptps_num=0;
struct Page_Table_Page **free_ptps;
uint64_t (*kernel_cr3)[512];
// 内核栈

static inline void mark(Memory_Block*const blocks, size_t *const blocks_num, const size_t index, const uint64_t start, const uint64_t end, const uint32_t type);
static inline void* malloc_mark(size_t const size, size_t const p2align, const uint32_t type, Memory_Block *const blocks, size_t *const blocks_num);
static inline __attribute__((noreturn)) void error();

static inline uint64_t (*)[512] new_pt();
static inline void init_ptp(struct Page_Table_Page*const);
static inline void map_page_2m(const uint64_t v_page, const uint64_t phy_page, uint64_t (*const cr3)[512]);
#define MAX_BLOCKS_NUM 512

/* virtual memory map:
 * 0-4G kernel text data rodata free_pages
 * 4G-30T kernel malloc
 * 30T-32T kernel stack
 * 32T-64T user data text rodata bss
 * 64T-248T user malloc
 * 248T-256T user stack
 * */

struct Page_Table_Page __attribite__((aligned(1<<21)))
{
    uint64_t pt[511][512] __attribute__((aligned(packed)));
    uint16_t free_pts_num;
    uint16_t free_pts[511];
    uint16_t pte_nums[511];
};

__attribute__((noreturn))
void main(const Memory_Block *const blocks, const size_t blocks_num)
{
    if ( GET_BITS_LOW((uint64_t)_start, 5) != 0 )
    {
        // 内核加载位置aligned不足
        error();
    }


    // 获取cpu逻辑核心数量
    cores_num=8;

    // type:0 空闲块
    //1 
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


    // init page
    {
        compiletime_assert(sizeof(struct Page_Table_Page)==((size_t)1<<21), "invalid struct Page_Table_Page!");
        struct Page_Table_Page* ptp=malloc_mark((size_t)1<<21, 21, 2, usable_blocks, &usable_blocks_num);
        init_page_table_page(ptp);
        
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

inline void map_page_2m(const uint64_t v_page, const uint64_t phy_page, uint64_t (*const cr3)[512])
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
    uint64_t i=v_page>>39;
    if ( (*cr3)[i] == 0 )
    {
        struct Page_Table_Page *const ptp=(struct Page_Table_Page*)REMOVE_BITS_LOW((uint64_t)cr3, 21);
        size_t i_in_ptp=GET_BITS_LOW((uint64_t)cr3, 21)>>12;
        ++ptp->valid_num[i_in_ptp];
        uint64_t (*const pt)[512]=new_pt();
        (*cr3)[i]=(uint64_t)pt|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2);
    }
    uint64_t (*const pt1)[512]=(uint64_t (*)[512])REMOVE_BITS_LOW((*cr3)[i], 12);
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

inline uint64_t (*)[512] new_pt()
{
    if ( free_ptps_num != 0 )
    {
        struct Page_Table_Page *const chosen_ptp=free_ptps[0];
        uint64_t (*const pt)[512]=chosen_ptp->pt[chosen_ptp->free_pts[--chosen_ptp->free_pts_num]];
        if ( chosen_ptp->free_pts_num == 0 )
        {
            ++free_ptps;
            --free_ptps_num;
        }
        return pt;
    }
    else
    {
        if ( free_pages_num == 0 )
        {
            error();
        }
        struct Page_Table_Page *const new_ptp=free_pages[--free_pages_num];
        init_ptp(new_ptp);
        uint64_t (*const pt)[512]=new_ptp->pt[new_ptp->free_pts[--new_ptp->free_pts_num]];
        --free_ptps;
        free_ptps[0]=new_ptp;
        ++free_ptps_num;
        return pt;
    }
}
inline void init_ptp(struct Page_Table_Page*const ptp)
{
    *ptp=(struct Page_Table_Page){ {{0}}, 511, {511, 510, 509, 508, 507, 506, 505, 504, 503, 502, 501, 500, 499, 498, 497, 496, 495, 494, 493, 492, 491, 490, 489, 488, 487, 486, 485, 484, 483, 482, 481, 480, 479, 478, 477, 476, 475, 474, 473, 472, 471, 470, 469, 468, 467, 466, 465, 464, 463, 462, 461, 460, 459, 458, 457, 456, 455, 454, 453, 452, 451, 450, 449, 448, 447, 446, 445, 444, 443, 442, 441, 440, 439, 438, 437, 436, 435, 434, 433, 432, 431, 430, 429, 428, 427, 426, 425, 424, 423, 422, 421, 420, 419, 418, 417, 416, 415, 414, 413, 412, 411, 410, 409, 408, 407, 406, 405, 404, 403, 402, 401, 400, 399, 398, 397, 396, 395, 394, 393, 392, 391, 390, 389, 388, 387, 386, 385, 384, 383, 382, 381, 380, 379, 378, 377, 376, 375, 374, 373, 372, 371, 370, 369, 368, 367, 366, 365, 364, 363, 362, 361, 360, 359, 358, 357, 356, 355, 354, 353, 352, 351, 350, 349, 348, 347, 346, 345, 344, 343, 342, 341, 340, 339, 338, 337, 336, 335, 334, 333, 332, 331, 330, 329, 328, 327, 326, 325, 324, 323, 322, 321, 320, 319, 318, 317, 316, 315, 314, 313, 312, 311, 310, 309, 308, 307, 306, 305, 304, 303, 302, 301, 300, 299, 298, 297, 296, 295, 294, 293, 292, 291, 290, 289, 288, 287, 286, 285, 284, 283, 282, 281, 280, 279, 278, 277, 276, 275, 274, 273, 272, 271, 270, 269, 268, 267, 266, 265, 264, 263, 262, 261, 260, 259, 258, 257, 256, 255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240, 239, 238, 237, 236, 235, 234, 233, 232, 231, 230, 229, 228, 227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198, 197, 196, 195, 194, 193, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180, 179, 178, 177, 176, 175, 174, 173, 172, 171, 170, 169, 168, 167, 166, 165, 164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0}, {0} };
}
