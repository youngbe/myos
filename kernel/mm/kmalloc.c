/* 这是一个可移植的malloc/free函数的实现，用于在分页的基础上进行更细致的内存管理
 * 适用于所有指令集和系统架构，只要系统使用分页的内存管理
 * 只需要指定 PAGE_P2SIZE MALLOC_P2ALIGNMENT _BASE _LIMIT 四个宏且操作系统实现了ALLOC_PAGES, FREE_PAGES 两个函数就可编译
 * 关于这几个宏的含义，请见后文说明
 */


#include <stdint.h>
#include <stddef.h>

#include <list.h>
#include <tools.h>
#include <bit.h>


// malloc返回的地址至少对齐2的多少次方
// 在 Linux x86_64 上是16==2^4
#define MALLOC_P2ALIGNMENT 4

// 页大小为2的多少次方
#define PAGE_P2SIZE 21

// 堆起始位置，含义见后文解释
// 4T
#define _BASE ((uintptr_t)0x40000000000)

// 堆的最大地址，含义见后文解释
// 30T
#define _LIMIT ((uintptr_t)(0x1e0000000000-1))

#define MALLOC kmalloc
#define FREE kfree
#define MALLOC_P2ALIGN kmalloc_p2align
// int ALLOC_PAGES(void* base, size_t num);
#define ALLOC_PAGES(base, num) alloc_pages_kernel(base, num)
int alloc_pages_kernel(void *const base, const size_t num);
// void FREE_PAGES(void* base, size_t num);
#define FREE_PAGES(base, num) free_pages_kernel(base, num)
void free_pages_kernel(void *const base, const size_t num);

#define MALLOC_ALIGNMENT (((uintptr_t)1)<<MALLOC_P2ALIGNMENT)
#define PAGE_SIZE (((uintptr_t)1)<<PAGE_P2SIZE)

typedef struct Block_Header Block_Header;
typedef struct Block Block;
struct __attribute__((aligned(MALLOC_ALIGNMENT))) Block_Header
{
    uintptr_t size;
    // == 0 空闲 ==1 已分配
    uint_fast8_t flag;
    Block *prev;
    struct list_head node_free_blocks;
};

struct __attribute__((aligned(MALLOC_ALIGNMENT))) Block
{
    Block_Header header;
    uint8_t content[] __attribute__((aligned(MALLOC_ALIGNMENT)));
};

static Block *last_block=NULL;
static struct list_head head_free_blocks={&head_free_blocks, &head_free_blocks};

/*
 * 数据结构说明：
 * 1. _BASE(类型为uintptr_t)是堆的起始地址
 * 2. 设堆的大小为size，则堆的范围为[_BASE, _BASE+size-1]，初始状态下堆大小为0
 * 3. size最大不会超过 (_LIMIT - _BASE + 1)
 * 4. 堆中放的是一个一个块(Block)，紧凑堆放
 * 5. 每个块的结构为：
 * struct
 * {
 * // 块头
 * Block_Header header;
 * // 块内容（地址返回给程序）
 * uint8_t content[header.size];
 * };
 * 6. 如果last_block为NULL，则代表堆大小为0，否则last_block指向堆的最后一个块
 * 7. 最后一个块的类型一定是已分配
 * 8. 不可能存在两个连续的未分配的块（两个连续的未分配块将会合并为一个块）
 * 9. free_blocks是一个连接起所有未分配块的链表
 *
 * 限制条件说明：
 * 1. 0 <= _BASE <_LIMIT <= UINTPTR_MAX
 * 2. _LIMIT - _BASE >= offsetof(Block, content)
 * 2. _BASE 对齐 MALLOC_ALIGNMENT 和 PAGE_SIZE
 * 3. [ _BASE, _LIMIT ] 不覆盖 NULL 和 (NULL + MALLOC_ALIGNMENT)
 * 4. _LIMIT <= UINTPTR_MAX - 页大小
 * 5. 0 < offsetof( Block, content ) < UINTPTR_MAX/2
 * 6. PAGE_P2SIZE < sizeof(uintptr_t)*8, MALLOC_P2ALIGNMENT < sizeof(uintptr_t)*8
 *
 * 本算法实现假设：
 * 1. sizeof(void *) == sizeof(uintptr_t)
 * 2. sizeof(uintptr_t) 可能大于或小于 sizeof(size_t)
 * 3. sizeof((uintptr_t)1 + (uintptr_t)1) 不一定等于 sizeof(uintptr_t)
 *    比如，在x86_64下，sizeof((short)1 +(short)1) == sizeof(int)
 */

// 下面这些宏定义仅允许作用于 uintptr_t ，返回uintptr_t
#define P2ALIGN(x, p2align) REMOVE_BITS_LOW((uintptr_t)(x), (p2align))
#define UP_P2ALIGN(x, p2align) ((uintptr_t)(REMOVE_BITS_LOW((uintptr_t)((uintptr_t)(x)-1), (p2align)) + (((uintptr_t)1)<<(p2align))))
#define ALIGN_PAGE(x) P2ALIGN((uintptr_t)(x), PAGE_P2SIZE)
#define UP_ALIGN_PAGE(x) ((uintptr_t)(ALIGN_PAGE((uintptr_t)((uintptr_t)(x)-1)) + PAGE_SIZE))
#define ALIGN_MALLOC(x) P2ALIGN((uintptr_t)(x), MALLOC_P2ALIGNMENT)
#define UP_ALIGN_MALLOC(x) ((uintptr_t)(ALIGN_MALLOC((uintptr_t)((uintptr_t)(x)-1)) + MALLOC_ALIGNMENT))

static inline int alloc_pages_tool( uintptr_t page_start, uintptr_t page_end );
static inline int alloc_pages_tool1( uintptr_t page_start, uintptr_t page_limit );
static inline int alloc_pages_tool1_( uintptr_t page_start, uintptr_t page_limit );
static inline void free_pages_tool( uintptr_t page_start, uintptr_t page_end );
static inline void free_pages_tool1( uintptr_t page_start, uintptr_t page_limit );
static inline void free_pages_tool1_( uintptr_t page_start, uintptr_t page_limit );

void *MALLOC(size_t size_o)
{
    compiletime_assert( offsetof(Block, content) < (UINTPTR_MAX>>1), "offsetof(Block, content) too large!" );
    compiletime_assert( offsetof(Block, content) != 0, "offsetof(Block, content) == 0!" );
    if ( size_o > UINTPTR_MAX )
    {
        return NULL;
    }
    uintptr_t size=size_o;
    if ( size == 0 )
    {
        return (void *)((uintptr_t)NULL+MALLOC_ALIGNMENT);
    }
    // size 向上取整对齐MALLOC_P2ALIGNMENT
    size=UP_ALIGN_MALLOC(size);
    if ( size == 0 )
    {
        return NULL;
    }
    // 先遍历一遍空闲块，看是否有合适的
    {
        Block *free_block;
        list_for_each_entry(free_block, &head_free_blocks, header.node_free_blocks)
        {
            if ( free_block->header.size<size )
            {
                continue;
            }
            uintptr_t const next_block_t=(uintptr_t)&free_block->content[free_block->header.size];
            if ( free_block->header.size - size <= offsetof(Block, content) )
            {
                // free aera : free_block->content[ 0 to (free_block->header.size-1) ]
                if ( alloc_pages_tool(UP_ALIGN_PAGE((uintptr_t)free_block->content), ALIGN_PAGE(next_block_t)) != 0 )
                {
                    return NULL;
                }
                free_block->header.flag=1;
                list_del(&free_block->header.node_free_blocks);
                return (void *)free_block->content;
            }
            else
            {
                // 增加块 new_block
                uintptr_t const new_content=next_block_t-size;
                Block *const new_block=(Block*)(uintptr_t)(new_content-offsetof(Block, content));
                {
                    uintptr_t page_start=ALIGN_PAGE((uintptr_t)new_block);
                    if ( page_start == ALIGN_PAGE((uintptr_t)free_block->content-1) )
                    {
                        page_start+=PAGE_SIZE;
                    }
                    if ( alloc_pages_tool(page_start, ALIGN_PAGE(next_block_t)) != 0 )
                    {
                        return NULL;
                    }
                }

                free_block->header.size=(uintptr_t)new_block-(uintptr_t)free_block->content;
                new_block->header.size=size;
                new_block->header.flag=1;
                new_block->header.prev=free_block;
                return (void *)new_content;
            }
        }
    }

    // 没有找到合适的空闲块，创建新的空闲块

    // 内存里目前一块都没有，从_BASE开始申请新的页表
    if ( last_block == NULL )
    {
        if ( size >= _LIMIT - _BASE )
        {
            return NULL;
        }
        if ( _LIMIT - _BASE - size < offsetof(Block, content) - 1 )
        {
            return NULL;
        }
        if ( ALLOC_PAGES((void *)_BASE, ((size+offsetof(Block, content)-1)>>PAGE_P2SIZE) +1 ) != 0 )
        {
            return NULL;
        }
        last_block=(Block *)_BASE;
        last_block->header.size=size;
        last_block->header.flag=1;
        //last_block->prev=NULL;
    }
    // 在堆结尾创建新的块
    else
    {
        if ( _LIMIT - (uintptr_t)&last_block->content[last_block->header.size-1] <= size )
        {
            return NULL;
        }
        if ( _LIMIT - (uintptr_t)&last_block->content[last_block->header.size-1] - size < offsetof(Block, content) )
        {
            return NULL;
        }
        Block *const new_block=(Block*)&last_block->content[last_block->header.size];
        if ( alloc_pages_tool1(UP_ALIGN_PAGE((uintptr_t)new_block), ALIGN_PAGE((uintptr_t)&new_block->content[size]-1)) != 0 )
        {
            return NULL;
        }
        new_block->header.size=size;
        new_block->header.flag=1;
        new_block->header.prev=last_block;
        last_block=new_block;
    }
    return (void *)last_block->content;
}

void FREE(void *base)
{
    Block *const del_block=(Block *)(uintptr_t)((uintptr_t)base-offsetof(Block, content));
    if ( del_block == last_block )
    {
        if ( (uintptr_t)del_block == _BASE )
        {
            // 没有更多的块了，清空堆
            FREE_PAGES((void*)_BASE, ( (del_block->header.size+offsetof(Block, content)-1) >> PAGE_P2SIZE)+1);
            last_block=NULL;
            return;
        }
        Block *const prev_block=del_block->header.prev;
        if ( prev_block->header.flag == 0 )
        {
            if ( (uintptr_t)prev_block == _BASE)
            {
                // 没有更多的块了，清空堆
                uintptr_t const page_start=ALIGN_PAGE((uintptr_t)del_block);
                if ( page_start - _BASE > PAGE_SIZE )
                {
                    // 有空隙
                    free_pages_tool1_(page_start, ALIGN_PAGE((uintptr_t)&del_block->content[del_block->header.size-1]));
                    FREE_PAGES((void*)_BASE, ((offsetof(Block, content)-1)>>PAGE_P2SIZE) +1);
                }
                else
                {
                    FREE_PAGES((void*)_BASE, ( ((uintptr_t)&del_block->content[del_block->header.size-1]-_BASE) >> PAGE_P2SIZE)+1);
                }
                head_free_blocks.prev=head_free_blocks.next=&head_free_blocks;
                last_block=NULL;
                return;
            }
            // 删除 del 块，prev 块
            // free del prev.header
            list_del(&prev_block->header.node_free_blocks);
            last_block=prev_block->header.prev;
            uintptr_t const page_start0=UP_ALIGN_PAGE((uintptr_t)prev_block);
            uintptr_t const page_limit0=ALIGN_PAGE((uintptr_t)prev_block->content-1);
            uintptr_t const page_start=ALIGN_PAGE((uintptr_t)del_block);
            uintptr_t const page_limit=ALIGN_PAGE((uintptr_t)&del_block->content[del_block->header.size-1]);
            if ( page_start - page_limit0 > PAGE_SIZE )
            {
                // 有空隙
                free_pages_tool1(page_start0, page_limit0);
                free_pages_tool1_(page_start, page_limit);
            }
            else
            {
                free_pages_tool1(page_start0, page_limit);
            }
        }
        else
        {
            // 删除del 块
            // free del
            last_block=prev_block;
            free_pages_tool1(UP_ALIGN_PAGE((uintptr_t)del_block), ALIGN_PAGE((uintptr_t)&del_block->content[del_block->header.size-1]));
        }
        return;
    }




    Block *const next_block=(Block *)((uintptr_t)base+del_block->header.size);
    if ( (uintptr_t)del_block == _BASE )
    {
        del_block->header.flag=0;
        if ( next_block->header.flag == 0 )
        {
            // 删除next块
            // free del next.header
            uintptr_t const next_next_block_t=(uintptr_t)&next_block->content[next_block->header.size];
            del_block->header.size=next_next_block_t-(uintptr_t)del_block->content;
            list_replace(&next_block->header.node_free_blocks, &del_block->header.node_free_blocks);
            uintptr_t const page_limit=ALIGN_PAGE((uintptr_t)next_block->content-1);
            if ( page_limit == ALIGN_PAGE(next_next_block_t) )
            {
                free_pages_tool(UP_ALIGN_PAGE((uintptr_t)del_block->content), page_limit);
            }
            else
            {
                free_pages_tool1(UP_ALIGN_PAGE((uintptr_t)del_block->content), page_limit);
            }
        }
        else
        {
            // free del.content
            list_add(&del_block->header.node_free_blocks, &head_free_blocks);
            free_pages_tool(UP_ALIGN_PAGE((uintptr_t)del_block->content), ALIGN_PAGE((uintptr_t)next_block) );
        }
        return;
    }

    Block *const prev_block=del_block->header.prev;
    if ( prev_block->header.flag == 0 )
    {
        uintptr_t page_start=ALIGN_PAGE((uintptr_t)del_block);
        if ( page_start == ALIGN_PAGE((uintptr_t)prev_block->content-1) )
        {
            page_start+=PAGE_SIZE;
        }
        if ( next_block->header.flag == 0 )
        {
            // 删除 del 块和next块
            // free del + next.header
            uintptr_t const next_next_block_t=(uintptr_t)&next_block->content[next_block->header.size];
            prev_block->header.size=next_next_block_t-(uintptr_t)prev_block->content;
            list_del(&next_block->header.node_free_blocks);
            uintptr_t const page_limit=ALIGN_PAGE((uintptr_t)next_block->content-1);
            if ( page_limit == ALIGN_PAGE(next_next_block_t) )
            {
                free_pages_tool(page_start, page_limit);
            }
            else
            {
                free_pages_tool1(page_start, page_limit);
            }
        }
        else
        {
            // 删除del 块
            // free del
            prev_block->header.size=(uintptr_t)next_block-(uintptr_t)prev_block->content;
            free_pages_tool(page_start, ALIGN_PAGE((uintptr_t)next_block));
        }
    }
    else
    {
        del_block->header.flag=0;
        if ( next_block->header.flag == 0 )
        {
            // 删除 next块
            // free del.content next.header
            list_replace(&next_block->header.node_free_blocks, &del_block->header.node_free_blocks);
            uintptr_t const next_next_block_t=(uintptr_t)&next_block->content[next_block->header.size];
            del_block->header.size=next_next_block_t-(uintptr_t)del_block->content;
            uintptr_t const page_limit=ALIGN_PAGE((uintptr_t)next_block->content-1);
            if ( page_limit == ALIGN_PAGE(next_next_block_t) )
            {
                free_pages_tool(UP_ALIGN_PAGE((uintptr_t)del_block->content), page_limit);
            }
            else
            {
                free_pages_tool1(UP_ALIGN_PAGE((uintptr_t)del_block->content), page_limit);
            }
        }
        else
        {
            // free del.content
            list_add(&del_block->header.node_free_blocks, &head_free_blocks);
            free_pages_tool(UP_ALIGN_PAGE((uintptr_t)del_block->content), ALIGN_PAGE((uintptr_t)next_block));
        }
    }
}

void *MALLOC_P2ALIGN(size_t size_o, const size_t p2align)
{
    if ( p2align <= MALLOC_P2ALIGNMENT )
    {
        return MALLOC(size_o);
    }
    if ( size_o > UINTPTR_MAX )
    {
        return NULL;
    }
    uintptr_t size=size_o;
    if ( size == 0 )
    {
        return (void *)((uintptr_t)NULL+MALLOC_ALIGNMENT);
    }
    // size 向上取整对齐MALLOC_P2ALIGNMENT
    size=UP_ALIGN_MALLOC(size);
    if ( size == 0 )
    {
        return NULL;
    }
    if ( p2align >= sizeof(uintptr_t)*8 )
    {
        return NULL;
    }
    uintptr_t const align=((uintptr_t)1)<<p2align;
    // 先遍历一遍空闲块，看是否有合适的
    {
        Block *free_block;
        list_for_each_entry(free_block, &head_free_blocks, header.node_free_blocks)
        {
            // prev_block free_block new_block new_free_block next_block
            uintptr_t const free_content=(uintptr_t)free_block->content;
            uintptr_t const next_block_t=free_content+free_block->header.size;
            if ( next_block_t - free_content < size )
            {
                continue;
            }
            if ( (uintptr_t)free_block == _BASE )
            {
                uintptr_t const new_content=P2ALIGN(next_block_t-size, p2align);
                if ( new_content < free_content )
                {
                    continue;
                }
                Block *const new_block=(Block *)(uintptr_t)(new_content-offsetof(Block, content));
                if ( (uintptr_t)new_block < free_content)
                {
                    if ( new_block != free_block )
                    {
                        continue;
                    }
                    if ( new_content+size+offsetof(Block, content) >= next_block_t )
                    {
                        goto label1;
                    }
                    else
                    {
                        goto label2;
                    }
                }
                if ( new_content+size+offsetof(Block, content) >= next_block_t )
                {
                    // 后面贴住
                    if ( new_block == free_block )
                    {
label1:
                        // 前面后面都贴住了
                        // alloc free_content to next_block_t-1
                        if (alloc_pages_tool(UP_ALIGN_PAGE(free_content), ALIGN_PAGE(next_block_t)) != 0)
                        {
                            return NULL;
                        }
                        list_del(&free_block->header.node_free_blocks);
                    }
                    else
                    {
                        // 前面没贴住，后面贴住了
                        {
                            // alloc new_block to next_block_t-1
                            uintptr_t page_start=ALIGN_PAGE((uintptr_t)new_block);
                            if ( page_start == ALIGN_PAGE(free_content-1) )
                            {
                                page_start+=PAGE_SIZE;
                            }
                            if ( alloc_pages_tool(page_start, ALIGN_PAGE(next_block_t)) != 0 )
                            {
                                return NULL;
                            }
                        }
                        free_block->header.size=(uintptr_t)new_block-free_content;
                        new_block->header.size=next_block_t-new_content;
                        new_block->header.prev=free_block;
                    }
                    new_block->header.flag=1;
                    return (void*)new_content;
                }
                else
                {
                    if ( GET_BITS_LOW(free_content, p2align) == 0 )
                    {
label2:
                        // 后面没贴住，前面贴住了
                        Block *const new_free_block=(Block*)(free_content+size);
                        uintptr_t const new_free_content=(uintptr_t)new_free_block->content;
                        {
                            uintptr_t const page_limit=ALIGN_PAGE(new_free_content-1);
                            if ( page_limit == ALIGN_PAGE(next_block_t) )
                            {
                                if ( alloc_pages_tool( UP_ALIGN_PAGE(free_content), page_limit ) != 0 )
                                {
                                    return NULL;
                                }
                            }
                            else
                            {
                                if ( alloc_pages_tool1( UP_ALIGN_PAGE(free_content), page_limit ) != 0 )
                                {
                                    return NULL;
                                }
                            }
                        }
                        list_replace(&free_block->header.node_free_blocks, &new_free_block->header.node_free_blocks);
                        free_block->header.size=size;
                        free_block->header.flag=1;
                        new_free_block->header.size=next_block_t-new_free_content;
                        new_free_block->header.flag=0;
                        new_free_block->header.prev=free_block;
                        return (void *)free_content;
                    }
                    else
                    {
                        // 前面后面都没贴住
                        Block *const new_free_block=(Block*)(uintptr_t)(new_content+size);
                        uintptr_t const new_free_content=(uintptr_t)new_free_block->content;
                        {
                            // alloc new_block to new_free_content-1
                            uintptr_t page_start=ALIGN_PAGE((uintptr_t)new_block);
                            if ( page_start == ALIGN_PAGE(free_content-1) )
                            {
                                page_start+=PAGE_SIZE;
                            }
                            uintptr_t const page_limit=ALIGN_PAGE(new_free_content-1);
                            if ( page_limit == ALIGN_PAGE(next_block_t) )
                            {
                                if ( alloc_pages_tool(page_start, page_limit) != 0 )
                                {
                                    return NULL;
                                }
                            }
                            else
                            {
                                if ( alloc_pages_tool1(page_start, page_limit) != 0 )
                                {
                                    return NULL;
                                }
                            }
                        }
                        list_add(&new_free_block->header.node_free_blocks, &head_free_blocks);
                        free_block->header.size=(uintptr_t)new_block-free_content;
                        new_block->header.size=size;
                        new_block->header.flag=1;
                        new_block->header.prev=free_block;
                        new_free_block->header.size=next_block_t-new_free_content;
                        new_free_block->header.flag=0;
                        new_free_block->header.prev=new_block;
                        return (void*)new_content;
                    }
                }
            }

            // 从后面贴
            {
                uintptr_t const new_content=P2ALIGN(next_block_t-size, p2align);
                if ( new_content < free_content )
                {
                    continue;
                }
                if ( new_content+size+offsetof(Block, content) < next_block_t )
                {
                    // 没贴住
                    goto label_next;
                }
                // 贴住了
                // alloc new_block to next_block-1
                Block *const new_block=(Block *)(uintptr_t)(new_content-offsetof(Block, content));
                if ( (uintptr_t)new_block <= free_content )
                {
                    // 同时也贴住了前面
                    // alloc free_content to next_block_t-1
                    if ( alloc_pages_tool(UP_ALIGN_PAGE(free_content), ALIGN_PAGE(next_block_t)) != 0 )
                    {
                        return NULL;
                    }
                    list_del(&free_block->header.node_free_blocks);
                    if ( new_block != free_block )
                    {
                        Block* prev_block=free_block->header.prev;
                        prev_block->header.size=(uintptr_t)new_block-(uintptr_t)prev_block->content;
                        new_block->header.size=next_block_t-new_content;
                        new_block->header.prev=prev_block;
                    }
                }
                else
                {
                    // 没能贴住前面
                    {
                        uintptr_t page_start=ALIGN_PAGE((uintptr_t)new_block);
                        if ( page_start == ALIGN_PAGE(free_content-1) )
                        {
                            page_start+=PAGE_SIZE;
                        }
                        if ( alloc_pages_tool(page_start, ALIGN_PAGE(next_block_t)) != 0 )
                        {
                            return NULL;
                        }
                    }
                    free_block->header.size=(uintptr_t)new_block-free_content;
                    new_block->header.size=next_block_t-new_content;
                    new_block->header.prev=free_block;
                }
                new_block->header.flag=1;
                return (void *)new_content;
            }
label_next:
            // 从前面贴
            uintptr_t const new_content=UP_P2ALIGN(free_content, p2align);
            Block *const new_block=(Block *)(uintptr_t)(new_content-offsetof(Block, content));
            Block *const new_free_block=(Block *)(uintptr_t)(new_content+size);
            uintptr_t const new_free_content=(uintptr_t)new_free_block->content;
            if ( (uintptr_t)new_block <= free_content )
            {
                // 贴住了
                // alloc free_content to new_free_content-1
                {
                    uintptr_t const page_limit=ALIGN_PAGE(new_free_content-1);
                    if ( page_limit == ALIGN_PAGE(next_block_t) )
                    {
                        if (alloc_pages_tool(UP_ALIGN_PAGE(free_content), page_limit) != 0)
                        {
                            return NULL;
                        }
                    }
                    else
                    {
                        if (alloc_pages_tool1(UP_ALIGN_PAGE(free_content), page_limit) != 0 )
                        {
                            return NULL;
                        }
                    }
                }
                list_replace(&free_block->header.node_free_blocks, &new_free_block->header.node_free_blocks);
                if ( free_block != new_block )
                {
                    Block *prev_block=free_block->header.prev;
                    prev_block->header.size=(uintptr_t)new_block-(uintptr_t)prev_block->content;
                    new_block->header.prev=prev_block;
                }
                new_block->header.size=size;
            }
            else
            {
                // alloc new_block to new_free_content-1
                {
                    uintptr_t page_start=ALIGN_PAGE((uintptr_t)new_block);
                    if ( page_start == ALIGN_PAGE(free_content-1) )
                    {
                        page_start+=PAGE_SIZE;
                    }
                    uintptr_t const page_limit=ALIGN_PAGE(new_free_content-1);
                    if ( page_limit == ALIGN_PAGE(next_block_t) )
                    {
                        if (alloc_pages_tool(page_start, page_limit) != 0)
                        {
                            return NULL;
                        }
                    }
                    else
                    {
                        if (alloc_pages_tool1(page_start, page_limit) != 0 )
                        {
                            return NULL;
                        }
                    }
                }
                list_add(&new_free_block->header.node_free_blocks, &head_free_blocks);
                free_block->header.size=(uintptr_t)new_block-free_content;
                new_block->header.size=size;
                new_block->header.prev=free_block;
            }
            new_block->header.flag=1;
            new_free_block->header.size=next_block_t-new_free_content;
            new_free_block->header.flag=0;
            new_free_block->header.prev=new_block;
            return (void *)new_content;
        }
    }

    // 没有找到合适的空闲块，创建新的空闲块

    // 内存里目前一块都没有，从_BASE开始申请新的页表
    if ( last_block == NULL )
    {
        if ( GET_BITS_LOW(_BASE+offsetof(Block, content), p2align) == 0 )
        {
            if ( size >= _LIMIT - _BASE )
            {
                return NULL;
            }
            if ( _LIMIT - _BASE - size < offsetof(Block, content) - 1 )
            {
                return NULL;
            }
            if ( ALLOC_PAGES((void*)_BASE, ((size+offsetof(Block, content)-1)>>PAGE_P2SIZE) +1 ) != 0 )
            {
                return NULL;
            }
            last_block=(Block *)_BASE;
            last_block->header.size=size;
            last_block->header.flag=1;
            return (void *)last_block->content;
        }
        else
        {
            if ( offsetof(Block, content)*2 > _LIMIT - _BASE )
            {
                return NULL;
            }
            uintptr_t new_content=P2ALIGN(_BASE+offsetof(Block, content)*2-1, p2align);
            if ( _LIMIT - new_content < align )
            {
                return NULL;
            }
            new_content+=align;
            if ( _LIMIT - new_content < size - 1 )
            {
                return NULL;
            }
            if ( ALLOC_PAGES((void*)_BASE, ((size+new_content-_BASE-1)>>PAGE_P2SIZE) +1 ) != 0 )
            {
                return NULL;
            }
            Block *const new_block=(Block*)(uintptr_t)(new_content-offsetof(Block, content));
            list_add(&last_block->header.node_free_blocks, &head_free_blocks);
            last_block=(Block *)_BASE;
            last_block->header.size=(uintptr_t)new_block-(uintptr_t)last_block->content;
            last_block->header.flag=0;
            new_block->header.size=size;
            new_block->header.flag=1;
            new_block->header.prev=last_block;
            return (void *)new_content;
        }
    }
    // 在堆结尾创建新的块
    if ( _LIMIT - (uintptr_t)&last_block->content[last_block->header.size-1] <= offsetof(Block, content) )
    {
        return NULL;
    }
    Block *const free_block=(Block*)&last_block->content[last_block->header.size];
    uintptr_t const free_content=(uintptr_t)free_block->content;
    uintptr_t new_content=P2ALIGN( free_content-1, p2align );
    if ( _LIMIT - new_content < align )
    {
        return NULL;
    }
    new_content+=align;
    if ( _LIMIT - new_content < size - 1 )
    {
        return NULL;
    }
    Block *const new_block=(Block *)(uintptr_t)(new_content-offsetof(Block, content));
    {
        uintptr_t const page_start0=UP_ALIGN_PAGE(free_block);
        uintptr_t const page_limit0=ALIGN_PAGE(free_content-1);
        uintptr_t const page_start=ALIGN_PAGE((uintptr_t)new_block);
        uintptr_t const page_limit=ALIGN_PAGE(new_content+size-1);
        if ( page_start - page_limit0 > PAGE_SIZE )
        {
            // 有空隙
            if ( alloc_pages_tool1( page_start0, page_limit0 ) != 0 )
            {
                return NULL;
            }
            if ( alloc_pages_tool1_( page_start, page_limit ) != 0 )
            {
                free_pages_tool1(page_start0, page_limit0);
                return NULL;
            }
        }
        else
        {
            if ( alloc_pages_tool1(page_start0, page_limit) != 0 )
            {
                return NULL;
            }
        }
    }
    new_block->header.size=size;
    new_block->header.flag=1;
    if ( (uintptr_t)new_block <= free_content )
    {
        // 贴住了
        if ( new_block != free_block )
        {
            last_block->header.size=(uintptr_t)new_block-(uintptr_t)last_block->content;
        }
        new_block->header.prev=last_block;
    }
    else
    {
        free_block->header.size=(uintptr_t)new_block-free_content;
        free_block->header.flag=0;
        free_block->header.prev=last_block;
        new_block->header.prev=free_block;
        list_add(&free_block->header.node_free_blocks, &head_free_blocks);
    }
    last_block=new_block;
    return (void *)new_content;
}

inline int alloc_pages_tool( uintptr_t page_start, uintptr_t page_end )
{
    if ( page_end > page_start )
    {
        return ALLOC_PAGES((void *)page_start, (page_end-page_start)>>PAGE_P2SIZE );
    }
    return 0;
}

inline int alloc_pages_tool1( uintptr_t page_start, uintptr_t page_limit )
{
    if ( page_limit >= page_start )
    {
        return ALLOC_PAGES((void *)page_start, ((page_limit-page_start)>>PAGE_P2SIZE)+1 );
    }
    return 0;
}

inline int alloc_pages_tool1_( uintptr_t page_start, uintptr_t page_limit )
{
    return ALLOC_PAGES((void *)page_start, ((page_limit-page_start)>>PAGE_P2SIZE)+1 );
}

inline void free_pages_tool( uintptr_t page_start, uintptr_t page_end )
{
    if ( page_end > page_start )
    {
        FREE_PAGES((void *)page_start, (page_end-page_start)>>PAGE_P2SIZE );
    }
}

inline void free_pages_tool1( uintptr_t page_start, uintptr_t page_limit )
{
    if ( page_limit >= page_start )
    {
        FREE_PAGES((void *)page_start, ((page_limit-page_start)>>PAGE_P2SIZE)+1 );
    }
}

inline void free_pages_tool1_( uintptr_t page_start, uintptr_t page_limit )
{
    FREE_PAGES((void *)page_start, ((page_limit-page_start)>>PAGE_P2SIZE)+1 );
}
