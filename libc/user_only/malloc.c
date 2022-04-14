#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include <list.h>
#include <bit.h>


// malloc返回的地址至少对齐2的多少次方
// 在 Linux x86_64 上是16==2^4
#define MALLOC_P2ALIGN 4

// 页大小为2的多少次方
#define PAGE_P2SIZE 21

// 堆起始位置，含义见后文解释
#define _BASE ((size_t)0x100000000000)

#define _LIMIT ((size_t)0x1000000000000)

typedef struct Block_Header Block_Header;
typedef struct Block Block;
struct __attribute__((aligned(_ALIGN))) Block_Header
{
    size_t size;
    // == 0 空闲 ==1 已分配
    size_t flag;
    Block *prev;
    struct list_head node_free_blocks;
};

struct __attribute__((aligned(_ALIGN))) Block
{
    Block_Header header;
    uint8_t content[] __attribute__((aligned(_ALIGN)));
};

static Block *last_block=NULL;
static struct list_head head_free_blocks={&head_free_blocks, &head_free_blocks};

/*
 * 数据结构说明：
 * 1. _BASE(类型为size_t)是堆的起始地址，_BASE必须对齐PAGE_SIZE和MALLOC_ALIGN，且不等于0
 * 2. 设堆的大小为size，则堆的范围为[_BASE, _BASE+size-1]，初始状态下堆大小为0
 * 3. 
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
 */

// 通过系统调用，申请/释放页表
// 由操作系统内核提供这两个函数的实现
ssize_t alloc_pages(void *base, size_t num);
void free_pages(void *base, size_t num);

#define MALLOC_ALIGN (((size_t)1)<<MALLOC_P2ALIGN)
#define PAGE_SIZE (((size_t)1)<<PAGE_P2SIZE)

#define P2ALIGN(x, p2align) REMOVE_BITS_LOW((x), (p2align))
#define UP_P2ALIGN(x, p2align) (REMOVE_BITS_LOW((x)-1, (p2align)) + (((size_t)1)<<(p2align)))
#define ALIGN_PAGE(x) P2ALIGN((x), PAGE_P2SIZE)
#define UP_ALIGN_PAGE(x) (ALIGN_PAGE((x)-1) + PAGE_SIZE)
#define ALIGN_MALLOC(x) P2ALIGN((x), MALLOC_P2ALIGN)
#define UP_ALIGN_MALLOC(x) (ALIGN_MALLOC((x)-1)+MALLOC_ALIGN)

static inline ssize_t alloc_pages_tool( size_t page_start, size_t page_end );
static inline ssize_t alloc_pages_tool1( size_t page_start, size_t page_limit );
static inline ssize_t alloc_pages_tool1_( size_t page_start, size_t page_limit );
static inline void free_pages_tool( size_t page_start, size_t page_end );
static inline void free_pages_tool1( size_t page_start, size_t page_limit );
static inline void free_pages_tool1_( size_t page_start, size_t page_limit );

void *malloc(size_t size)
{
    if ( size == 0 )
    {
        return (void *)((size_t)NULL+MALLOC_ALIGN);
    }
    // size 向上取整对齐MALLOC_P2ALIGN
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
            size_t const next_block_t=(size_t)&free_block->content[free_block->header.size];
            if ( free_block->header.size - size <= offsetof(Block, content) )
            {
                // free aera : free_block->content[ 0 to (free_block->header.size-1) ]
                if (
                        alloc_pages_tool(
                            UP_ALIGN_PAGE((size_t)free_block->content),
                            ALIGN_PAGE(next_block_t)
                            ) != 0 )
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
                size_t const new_content=next_block_t-size;
                Block *const new_block=(Block*)(new_content-offsetof(Block, content));
                {
                    size_t page_start=ALIGN_PAGE((size_t)new_block);
                    if ( page_start == ALIGN_PAGE((size_t)free_block->content-1) )
                    {
                        page_start+=PAGE_SIZE;
                    }
                    if ( alloc_pages_tool(page_start, ALIGN_PAGE(next_block_t)) != 0 )
                    {
                        return NULL;
                    }
                }

                free_block->header.size=(size_t)new_block-(size_t)free_block->content;
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
        if ( alloc_pages((void *)_BASE, ((size+offsetof(Block, content)-1)>>PAGE_P2SIZE) +1 ) != 0 )
        {
            return NULL;
        }
        last_block=(Block *)_BASE;
        last_block->header.size=size;
        last_block->header.flag=1;
        //last_block->prev=NULL;
        return (void *)last_block->content;
    }
    // 在堆结尾创建新的块
    else
    {
        if ( _LIMIT - (size_t)&last_block->content[last_block->header.size-1] <= size )
        {
            return NULL;
        }
        if ( _LIMIT - (size_t)&last_block->content[last_block->header.size-1] - size < offsetof(Block, content) )
        {
            return NULL;
        }
        Block *const new_block=(Block*)&last_block->content[last_block->header.size];
        if ( alloc_pages_tool1(
                    UP_ALIGN_PAGE((size_t)new_block),
                    ALIGN_PAGE((size_t)&new_block->content[size]-1)
                    ) != 0 )
        {
            return NULL;
        }
        new_block->header.size=size;
        new_block->header.flag=1;
        new_block->header.prev=last_block;
        last_block=new_block;
        return (void *)new_block->content;
    }
}

void free(void *base)
{
    Block *const del_block=(Block *)((size_t)base-offsetof(Block, content));
    if ( del_block == last_block )
    {
        if ( (size_t)del_block == _BASE )
        {
            // 没有更多的块了，清空堆
            free_pages((void*)_BASE, ( (del_block->header.size+offsetof(Block, content)-1) >> PAGE_P2SIZE)+1);
            last_block=NULL;
            return;
        }
        Block *const prev_block=del_block->header.prev;
        if ( prev_block->header.flag == 0 )
        {
            if ( (size_t)prev_block == _BASE)
            {
                // 没有更多的块了，清空堆
                size_t const page_start=ALIGN_PAGE((size_t)del_block);
                if ( page_start - _BASE > PAGE_SIZE )
                {
                    // 有空隙
                    free_pages_tool1_(page_start, ALIGN_PAGE((size_t)&del_block->content[del_block->header.size-1]);
                    free_pages((void*)_BASE, (((size_t)offsetof(Block, content)-1)>>PAGE_P2SIZE) +1);
                }
                else
                {
                    free_pages((void*)_BASE, ( ((size_t)&del_block->content[del_block->header.size-1]-_BASE) >> PAGE_P2SIZE)+1);
                }
                head_free_blocks.prev=head_free_blocks.next=&head_free_blocks;
                last_block=NULL;
                return;
            }
            // 删除 del 块，prev 块
            // free del prev.header
            list_del(&prev_block->header.node_free_blocks);
            last_block=prev_block->header.prev;
            size_t const page_start0=UP_ALIGN_PAGE((size_t)prev_block);
            size_t const page_limit0=ALIGN_PAGE((size_t)prev_block->content-1);
            size_t const page_start=ALIGN_PAGE((size_t)del_block);
            size_t const page_limit=ALIGN_PAGE((size_t)&del_block->content[del_block->header.size-1]);
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
            free_pages_tool1(UP_ALIGN_PAGE((size_t)del_block), ALIGN_PAGE((size_t)&del_block->content[del_block->header.size-1]));
        }
        return;
    }




    Block *const next_block=(Block *)((size_t)base+del_block->header.size);
    if ( (size_t)del_block == _BASE )
    {
        del_block->header.flag=0;
        if ( next_block->header.flag == 0 )
        {
            // 删除next块
            // free del next.header
            size_t const next_next_block_t=(size_t)&next_block->content[next_block->header.size];
            del_block->header.size=next_next_block_t-(size_t)del_block->content;
            list_replace(&next_block->header.node_free_blocks, &del_block->header.node_free_blocks);
            size_t const page_limit=ALIGN_PAGE((size_t)next_block->content-1);
            if ( page_limit == ALIGN_PAGE(next_next_block_t) )
            {
                free_pages_tool(UP_ALIGN_PAGE((size_t)del_block->content), page_limit);
            }
            else
            {
                free_pages_tool1(UP_ALIGN_PAGE((size_t)del_block->content), page_limit);
            }
        }
        else
        {
            // free del.content
            list_add(&del_block->header.node_free_blocks, &head_free_blocks);
            free_pages_tool(UP_ALIGN_PAGE((size_t)del_block->content), ALIGN_PAGE((size_t)next_block) );
        }
        return;
    }

    Block *const prev_block=del_block->header.prev;
    if ( prev_block->header.flag == 0 )
    {
        size_t page_start=ALIGN_PAGE((size_t)del_block);
        if ( page_start == ALIGN_PAGE((size_t)prev_block->content-1) )
        {
            page_start+=PAGE_SIZE;
        }
        if ( next_block->header.flag == 0 )
        {
            // 删除 del 块和next块
            // free del + next.header
            size_t const next_next_block_t=(size_t)&next_block->content[next_block->header.size];
            prev_block->header.size=next_next_block_t-(size_t)prev_block->content;
            list_del(&next_block->header.node_free_blocks);
            size_t const page_limit=ALIGN_PAGE((size_t)next_block->content-1);
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
            prev_block->header.size=(size_t)next_block-(size_t)prev_block->content;
            free_pages_tool(page_start, ALIGN_PAGE((size_t)next_block));
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
            size_t const next_next_block_t=(size_t)&next_block->content[next_block->header.size];
            del_block->header.size=next_next_block_t-(size_t)del_block->content;
            size_t const page_limit=ALIGN_PAGE((size_t)next_block->content-1);
            if ( page_limit == ALIGN_PAGE(next_next_block_t) )
            {
                free_pages_tool(UP_ALIGN_PAGE((size_t)del_block->content), page_limit);
            }
            else
            {
                free_pages_tool1(UP_ALIGN_PAGE((size_t)del_block->content), page_limit);
            }
        }
        else
        {
            // free del.content
            list_add(&del_block->header.node_free_blocks, &head_free_blocks);
            free_pages_tool(UP_ALIGN_PAGE((size_t)del_block->content), ALIGN_PAGE((size_t)next_block));
        }
    }
}

void *malloc_p2align(size_t size, const size_t p2align)
{
    if ( p2align <= _P2ALIGN )
    {
        return malloc(size);
    }
    if ( size == 0 )
    {
        return (void *)((size_t)NULL+MALLOC_ALIGN);
    }
    // size 向上取整对齐MALLOC_P2ALIGN
    size=UP_ALIGN_MALLOC(size);
    if ( size == 0 )
    {
        return NULL;
    }
    if ( p2align >= sizeof(size_t)*8 )
    {
        return NULL;
    }
    size_t const align=((size_t)1)<<p2align;
    // 先遍历一遍空闲块，看是否有合适的
    {
        Block *free_block;
        list_for_each_entry(free_block, &head_free_blocks, header.node_free_blocks)
        {
            // prev_block free_block new_block new_free_block next_block
            size_t const free_content=(size_t)free_block->content;
            size_t const next_block_t=free_content+free_block->header.size;
            if ( next_block_t - free_content < size )
            {
                continue;
            }
            if ( (size_t)free_block == _BASE )
            {
                size_t const new_content=P2ALIGN(next_block_t-size, p2align);
                if ( new_content < free_content )
                {
                    continue;
                }
                Block *const new_block=(Block *)(new_content-offsetof(Block, content));
                if ( (size_t)new_block < free_content)
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
                            size_t page_start=ALIGN_PAGE((size_t)new_block);
                            if ( page_start == ALIGN_PAGE(free_content-1) )
                            {
                                page_start+=PAGE_SIZE;
                            }
                            if ( alloc_pages_tool(page_start, ALIGN_PAGE(next_block_t)) != 0 )
                            {
                                return NULL;
                            }
                        }
                        free_block->header.size=(size_t)new_block-free_content;
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
                        Block *const new_free_block=free_content+size;
                        size_t const new_free_content=(size_t)new_free_block->content;
                        {
                            size_t const page_limit=ALIGN_PAGE(new_free_content-1);
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
                        Block *const new_free_block=new_content+size;
                        size_t const new_free_content=(size_t)new_free_block->content;
                        {
                            // alloc new_block to new_free_content-1
                            size_t page_start=ALIGN_PAGE((size_t)new_block);
                            if ( page_start == ALIGN_PAGE(free_content-1) )
                            {
                                page_start+=PAGE_SIZE;
                            }
                            size_t const page_limit=ALIGN_PAGE(new_free_content-1);
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
                        free_block->header.size=(size_t)new_block-free_content;
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
                size_t const new_content=P2ALIGN(next_block_t-size, p2align);
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
                Block *const new_block=(Block *)(new_content-offsetof(Block, content));
                if ( (size_t)new_block <= free_content )
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
                        Block* prev_block=free_block->prev;
                        prev_block->header.size=(size_t)new_block-(size_t)prev_block->content;
                        new_block->header.size=next_block_t-new_content;
                        new_block->header.prev=prev_block;
                    }
                }
                else
                {
                    // 没能贴住前面
                    {
                        size_t page_start=ALIGN_PAGE((size_t)new_block);
                        if ( page_start == ALIGN_PAGE(free_content-1) )
                        {
                            page_start+=PAGE_SIZE;
                        }
                        if ( alloc_pages_tool(page_start, ALIGN_PAGE(next_block_t)) != 0 )
                        {
                            return NULL;
                        }
                    }
                    free_block->header.size=(size_t)new_block-free_content;
                    new_block->header.size=next_block_t-new_content;
                    new_block->header.prev=free_block;
                }
                new_block->header.flag=1;
                return (void *)new_content;
            }
label_next:
            // 从前面贴
            size_t const new_content=UP_P2ALIGN(free_content, p2align);
            Block *const new_block=(Block *)(new_content-offsetof(Block, content));
            Block *const new_free_block=(Block *)(new_content+size);
            size_t const new_free_content=(size_t)new_free_block->content;
            if ( (size_t)new_block <= free_content )
            {
                // 贴住了
                // alloc free_content to new_free_content-1
                {
                    size_t const page_limit=ALIGN_PAGE(new_free_content-1);
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
                    prev_block->header.size=(size_t)new_block-(size_t)prev_block->content;
                    new_block->header.prev=prev_block;
                }
                new_block->header.size=header.size;
            }
            else
            {
                // alloc new_block to new_free_content-1
                {
                    size_t page_start=ALIGN_PAGE((size_t)new_block);
                    if ( page_start == ALIGN_PAGE(free_content-1) )
                    {
                        page_start+=PAGE_SIZE;
                    }
                    size_t const page_limit=ALIGN_PAGE(new_free_content-1);
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
                free_block->header.size=(size_t)new_block-free_content;
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
            if ( alloc_pages((void*)_BASE, ((size+offsetof(Block, content)-1)>>PAGE_P2SIZE) +1 ) != 0 )
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
            if ( (size_t)offsetof(Block, content)*2 > _LIMIT - _BASE )
            {
                return NULL;
            }
            size_t new_content=P2ALIGN(_BASE+(size_t)offsetof(Block, content)*2-1, p2align);
            if ( _LIMIT - new_content < align )
            {
                return NULL;
            }
            new_content+=align;
            if ( _LIMIT - new_content < size - 1 )
            {
                return NULL;
            }
            if ( alloc_pages((void*)_BASE, ((size+new_content-_BASE-1)>>PAGE_P2SIZE) +1 ) != 0 )
            {
                return NULL;
            }
            Block *const new_block=(Block*)(new_content-offsetof(Block, content));
            list_add(&last_block->header.node_free_blocks, &head_fee_blocks);
            last_block=(Block *)_BASE;
            last_block->header.size=(size_t)new_block-(size_t)last_block->content;
            last_block->header.flag=0;
            new_block->header.size=size;
            new_block->header.flag=1;
            new_block->header.prev=last_block;
            return (void *)new_content;
        }
    }
    // 在堆结尾创建新的块
    if ( _LIMIT - (size_t)&last_block->content[last_block->header.size-1] <= offsetof(Block, content) )
    {
        return NULL;
    }
    Block *const free_block=(Block*)&last_block->content[last_block->header.size];
    size_t const free_content=(size_t)free_block->content;
    size_t new_content=P2ALIGN( free_content-1, p2align );
    if ( _LIMIT - new_content < align )
    {
        return NULL;
    }
    new_content+=align;
    if ( _LIMIT - new_content < size - 1 )
    {
        return NULL;
    }
    Block *const new_block=(Block *)(new_content-offsetof(Block, content));
    {
        size_t const page_start0=UP_ALIGN_PAGE(free_block);
        size_t const page_limit0=ALIGN_PAGE(free_content-1);
        size_t const page_start=ALIGN_PAGE((size_t)new_block);
        size_t const page_limit=ALIGN_PAGE(new_content+size-1);
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
    if ( (size_t)new_block <= free_content )
    {
        // 贴住了
        if ( new_block != free_block )
        {
            last_block->header.size=(size_t)new_block-(size_t)last_block->content;
        }
        new_block->header.prev=last_block;
    }
    else
    {
        free_block->header.size=(size_t)new_block-free_content;
        free_block->header.flag=0;
        free_block->header.prev=last_block;
        new_block->header.prev=free_block;
        list_add(&free_block->header.node_free_blocks, &head_free_blocks);
    }
    last_block=new_block;
    return (void *)new_content;
}

inline ssize_t alloc_pages_tool( size_t page_start, size_t page_end )
{
    if ( page_end > page_start )
    {
        return alloc_pages((void *)page_start, (page_end-page_start)>>PAGE_P2SIZE );
    }
    return 0;
}

inline ssize_t alloc_pages_tool1( size_t page_start, size_t page_limit )
{
    if ( page_limit >= page_start )
    {
        return alloc_pages((void *)page_start, ((page_limit-page_start)>>PAGE_P2SIZE)+1 );
    }
    return 0;
}

inline ssize_t alloc_pages_tool1_( size_t page_start, size_t page_limit )
{
    return alloc_pages((void *)page_start, ((page_limit-page_start)>>PAGE_P2SIZE)+1 );
}

inline void free_pages_tool( size_t page_start, size_t page_end )
{
    if ( page_end > page_start )
    {
        free_pages((void *)page_start, (page_end-page_start)>>PAGE_P2SIZE );
    }
}

inline void free_pages_tool1( size_t page_start, size_t page_limit )
{
    if ( page_limit >= page_start )
    {
        free_pages((void *)page_start, ((page_limit-page_start)>>PAGE_P2SIZE)+1 );
    }
}

inline void free_pages_tool1_( size_t page_start, size_t page_limit )
{
    free_pages((void *)page_start, ((page_limit-page_start)>>PAGE_P2SIZE)+1 );
}
