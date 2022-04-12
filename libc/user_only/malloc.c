#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include <list.h>
#include <bit.h>


// malloc 对齐2的多少次方
#define _P2ALIGN ((size_t)4)

// 页大小为2的多少次方
#define PAGE_P2SIZE ((size_t)21)

// malloc 对齐多少字节
#define _ALIGN (((size_t)1)<<_P2ALIGN)

// 页大小
#define PAGE_SIZE (((size_t)1)<<PAGE_P2SIZE)

// malloc 从 1TB 开始（虚拟地址）
#define _BASE ((void *)0x10000000000)

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
数据结构说明：
1. _BASE是堆开始的地方，_BASE必须>=PAGE_SIZE且对齐PAGE_SIZE
2. 设堆的大小为size，则堆的范围为[_BASE, _BASE+size-1]，初始状态下堆大小为0。
3. 堆中放的是一个一个块(Block)，紧凑堆放
4. 每个块的结构为：
struct
{
    // 块头
    Block_Header header;
    // 块内容（地址返回给程序）
    uint8_t content[header.size];
};
5. 如果last_block为NULL，则代表堆大小为0，否则last_block指向堆的最后一个块
6. 最后一个块的类型一定是已分配
7. 不可能存在两个连续的未分配的块（两个连续的未分配块将会合并为一个块）
8. free_blocks是一个连接起所有未分配块的链表
*/

// 由操作系统内核提供这两个函数的实现
ssize_t alloc_pages(void *base, size_t num);
void free_pages(void *base, size_t num);

static inline ssize_t alloc_pages_tool( size_t page_start, size_t page_end );
static inline ssize_t alloc_pages_tool1( size_t page_start, size_t page_limit );
static inline void free_pages_tool( size_t page_start, size_t page_end );
static inline void free_pages_tool1( size_t page_start, size_t page_limit );

void *malloc(size_t size)
{
    if ( size == 0 )
    {
        return (void *)((size_t)NULL+_ALIGN);
    }
    // size 向上取整对齐_P2ALIGN
    size=REMOVE_BITS_LOW(size-1, _P2ALIGN)+_ALIGN;
    // >= 2^48
    if ( REMOVE_BITS_LOW(size, 48) !=0 )
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
            if ( free_block->header.size <= size+offsetof(Block, content) )
            {
                // free aera : free_block->content[ 0 to (free_block->header.size-1) ]
                if (
                        alloc_pages_tool(
                            REMOVE_BITS_LOW( (size_t)free_block->content-1 , PAGE_P2SIZE) + PAGE_SIZE,
                            REMOVE_BITS_LOW( next_block_t , PAGE_P2SIZE)
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
                size_t const new_start_t=next_block_t-size;
                Block *const new_block=(Block*)(new_start_t-offsetof(Block, content));
                {
                    size_t temp_page_start=REMOVE_BITS_LOW((size_t)new_block, PAGE_P2SIZE);
                    if ( temp_page_start == REMOVE_BITS_LOW((size_t)free_block->content-1, PAGE_P2SIZE) )
                    {
                        temp_page_start+=PAGE_SIZE;
                    }
                    if ( alloc_pages_tool(temp_page_start, REMOVE_BITS_LOW(next_block_t, PAGE_P2SIZE)) != 0 )
                    {
                        return NULL;
                    }
                }

                free_block->header.size=(size_t)new_block-(size_t)free_block->content;
                new_block->header.size=size;
                new_block->header.flag=1;
                new_block->header.prev=free_block;
                return (void *)new_start_t;
            }
        }
    }

    // 没有找到合适的空闲块，创建新的空闲块

    // 内存里目前一块都没有，从_BASE开始申请新的页表
    if ( last_block == NULL )
    {
        if ( alloc_pages(_BASE, ((size+offsetof(Block, content)-1)>>PAGE_P2SIZE) +1 ) != 0 )
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
        Block *const new_block=(Block*)&last_block->content[last_block->header.size];
        if ( alloc_pages_tool1(
                    REMOVE_BITS_LOW((size_t)new_block-1, PAGE_P2SIZE)+PAGE_SIZE,
                    REMOVE_BITS_LOW((size_t)&new_block->content[size]-1, PAGE_P2SIZE)
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
        if ( del_block == _BASE )
        {
            // 没有更多的块了，清空堆
            free_pages(_BASE, ( (del_block->header.size+offsetof(Block, content)-1) >> PAGE_P2SIZE)+1);
            last_block=NULL;
            return;
        }
        Block *const prev_block=del_block->header.prev;
        if ( prev_block->header.flag == 0 )
        {
            if ( prev_block == _BASE)
            {
                // 没有更多的块了，清空堆
                free_pages(_BASE, ( ((size_t)&del_block->content[del_block->header.size-1]-(size_t)_BASE) >> PAGE_P2SIZE)+1);
                head_free_blocks.prev=head_free_blocks.next=&head_free_blocks;
                last_block=NULL;
                return;
            }
            // 删除 del 块，prev 块
            // free del prev.header
            list_del(&prev_block->header.node_free_blocks);
            last_block=prev_block->header.prev;
            size_t free_page_start=REMOVE_BITS_LOW((size_t)del_block, PAGE_P2SIZE);
            size_t const prev_header_page=REMOVE_BITS_LOW((size_t)prev_block->content-1, PAGE_P2SIZE);
            if ( GET_BITS_LOW((size_t)prev_block->content-1, PAGE_P2SIZE) < offsetof(Block, content) )
            {
                if ( free_page_start - prev_header_page <= PAGE_SIZE )
                {
                    free_pages_tool1(prev_header_page, REMOVE_BITS_LOW((size_t)&del_block->content[del_block->header.size-1], PAGE_P2SIZE));
                }
                else
                {
                    free_pages((void *)prev_header_page, 1);
                    free_pages_tool1(free_page_start, REMOVE_BITS_LOW((size_t)&del_block->content[del_block->header.size-1], PAGE_P2SIZE));
                }
            }
            else
            {
                if ( free_page_start == prev_header_page )
                {
                    free_page_start+=PAGE_SIZE;
                }
                free_pages_tool1(free_page_start, REMOVE_BITS_LOW((size_t)&del_block->content[del_block->header.size-1], PAGE_P2SIZE));
            }
        }
        else
        {
            // 删除del 块
            // free del
            last_block=prev_block;
            free_pages_tool1(REMOVE_BITS_LOW((size_t)del_block-1, PAGE_P2SIZE)+PAGE_SIZE, REMOVE_BITS_LOW((size_t)&del_block->content[del_block->header.size-1], PAGE_P2SIZE));
        }
        return;
    }




    Block *const next_block=(Block *)((size_t)base+del_block->header.size);
    if ( del_block == _BASE )
    {
        del_block->header.flag=0;
        if ( next_block->header.flag == 0 )
        {
            // 删除next块
            // free del next.header
            size_t const next_next_block_t=(size_t)&next_block->content[next_block->header.size];
            del_block->header.size=next_next_block_t-(size_t)del_block->content;
            list_replace(&next_block->header.node_free_blocks, &del_block->header.node_free_blocks);
            size_t free_page_limit=REMOVE_BITS_LOW((size_t)next_block->content-1, PAGE_P2SIZE);
            if ( free_page_limit == REMOVE_BITS_LOW(next_next_block_t, PAGE_P2SIZE) )
            {
                free_pages_tool((size_t)_BASE+PAGE_SIZE, free_page_limit);
            }
            else
            {
                free_pages_tool1((size_t)_BASE+PAGE_SIZE, free_page_limit);
            }
        }
        else
        {
            // free del.content
            list_add(&del_block->header.node_free_blocks, &head_free_blocks);
            free_pages_tool((size_t)_BASE+PAGE_SIZE, REMOVE_BITS_LOW((size_t)next_block, PAGE_P2SIZE) );
        }
        return;
    }

    Block *const prev_block=del_block->header.prev;
    if ( prev_block->header.flag == 0 )
    {
        size_t free_page_start=REMOVE_BITS_LOW((size_t)del_block, PAGE_P2SIZE);
        if ( free_page_start == REMOVE_BITS_LOW((size_t)prev_block->content-1, PAGE_P2SIZE) )
        {
            free_page_start+=PAGE_SIZE;
        }
        if ( next_block->header.flag == 0 )
        {
            // 删除 del 块和next块
            // free del + next.header
            size_t const next_next_block_t=(size_t)&next_block->content[next_block->header.size];
            prev_block->header.size=next_next_block_t-(size_t)prev_block->content;
            list_del(&next_block->header.node_free_blocks);
            size_t const free_page_limit=REMOVE_BITS_LOW((size_t)next_block->content-1, PAGE_P2SIZE);
            if ( free_page_limit == REMOVE_BITS_LOW(next_next_block_t, PAGE_P2SIZE) )
            {
                free_pages_tool(free_page_start, free_page_limit);
            }
            else
            {
                free_pages_tool1(free_page_start, free_page_limit);
            }
        }
        else
        {
            // 删除del 块
            // free del
            prev_block->header.size=(size_t)next_block-(size_t)prev_block->content;
            free_pages_tool(free_page_start, REMOVE_BITS_LOW((size_t)next_block, PAGE_P2SIZE));
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
            size_t const free_pages_limit=REMOVE_BITS_LOW((size_t)next_block->content-1, PAGE_P2SIZE);
            if ( free_pages_limit == REMOVE_BITS_LOW(next_next_block_t, PAGE_P2SIZE) )
            {
                free_pages_tool(REMOVE_BITS_LOW((size_t)del_block->content-1, PAGE_P2SIZE)+PAGE_SIZE, free_pages_limit);
            }
            else
            {
                free_pages_tool1(REMOVE_BITS_LOW((size_t)del_block->content-1, PAGE_P2SIZE)+PAGE_SIZE, free_pages_limit);
            }
        }
        else
        {
            // free del.content
            list_add(&del_block->header.node_free_blocks, &head_free_blocks);
            free_pages_tool(
                    REMOVE_BITS_LOW((size_t)del_block->content-1, PAGE_P2SIZE)+PAGE_SIZE,
                    REMOVE_BITS_LOW((size_t)next_block, PAGE_P2SIZE)
                    );
        }
    }
}

inline ssize_t alloc_pages_tool( size_t page_start, size_t page_end )
{
    if ( page_end > page_start )
    {
        return alloc_pages((void *)page_start, (page_end-page_start)>>PAGE_P2SIZE );
    }
    return 0;
}

void *malloc_p2align(size_t size, const size_t p2align)
{
    if ( p2align <= _P2ALIGN )
    {
        return malloc(size);
    }
    if ( size == 0 )
    {
        return (void *)((size_t)NULL+_ALIGN);
    }
    // size 向上取整对齐_P2ALIGN
    size=REMOVE_BITS_LOW(size-1, _P2ALIGN)+_ALIGN;
    // >= 2^48
    if ( REMOVE_BITS_LOW(size, 48) !=0 || p2align >= 48 )
    {
        return NULL;
    }
    const size_t align=((size_t)1)<<p2align;
    // 先遍历一遍空闲块，看是否有合适的
    {
        Block *free_block;
        list_for_each_entry(free_block, &head_free_blocks, header.node_free_blocks)
        {
            // prev_block free_block insert_block ifree_block next_block
            size_t const free_content=(size_t)free_block->content;
            size_t const insert_content=REMOVE_BITS_LOW(free_content-1, p2align)+align;
            Block *const insert_block=(Block *)(insert_content-offsetof(Block, content));
            Block *const ifree_block= (Block *)(insert_content + size);
            size_t const ifree_content=(size_t)ifree_block->content
            size_t const next_block_t=free_content+free_block->header.size;
            Block *const prev_block=free_block->header.prev;
            size_t const prev_content=(size_t)prev_block->content;
            if ( next_block_t < ifree_block )
            {
                continue;
            }
            if ( (size_t)insert_block <= free_content )
            {
                if ( next_block_t <= ifree_content )
                {
                }
                else
                {
                }
            }
            else
            {
                if ( next_block_t <= ifree_content )
                {
                }
                else
                {
                }
            }
        }
    }

    // 没有找到合适的空闲块，创建新的空闲块

    // 内存里目前一块都没有，从_BASE开始申请新的页表
    if ( last_block == NULL )
    {
        if ( alloc_pages(_BASE, ((size+offsetof(Block, content)-1)>>PAGE_P2SIZE) +1 ) != 0 )
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
        Block *const new_block=(Block*)&last_block->content[last_block->header.size];
        if ( alloc_pages_tool1(
                    REMOVE_BITS_LOW((size_t)new_block-1, PAGE_P2SIZE)+PAGE_SIZE,
                    REMOVE_BITS_LOW((size_t)&new_block->content[size]-1, PAGE_P2SIZE)
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

inline ssize_t alloc_pages_tool1( size_t page_start, size_t page_limit )
{
    if ( page_limit >= page_start )
    {
        return alloc_pages((void *)page_start, ((page_limit-page_start)>>PAGE_P2SIZE)+1 );
    }
    return 0;
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
