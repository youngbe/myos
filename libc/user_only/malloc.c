#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include <list.h>
#include <bit.h>
// mmo 的头文件
//#include <>

// malloc 从 1TB 开始（虚拟地址）
#define _BASE ((void *)0x10000000000)

typedef struct Block_Header Block_Header;
typedef struct Block Block;
struct Block_Header
{
    size_t size;
    // == 0 空闲 ==1 已分配
    size_t flag;
    Block *prev;
    struct list_head node_free_blocks;
};

struct __attribute__((packed)) Block
{
    Block_Header header;
    uint8_t content[];
};

static Block *last_block=NULL;
static struct list_head head_free_blocks={&head_free_blocks, &head_free_blocks};

/*
数据结构说明：
1. _BASE是堆开始的地方，_BASE必须>=2M且对齐2M
2. 设堆的大小为size，则堆的范围为[_BASE, _BASE+size-1]，初始状态下堆大小为0。
3. 堆中放的是一个一个块(Block)，紧凑堆放
4. 每个块的结构为：
struct __attribute__((packed))
{
    Block_Header header;
    uint8_t content[header.size];
};
5. Block_Header中每个字段的含义非常好理解，看名字就行
6. 如果last_block为NULL，则代表堆大小为0，否则last_block指向堆的最后一个块
7. 最后一个块的类型一定是已分配
8. 不可能存在两个连续的未分配的块（两个连续的未分配块将会合并为一个块）
9. free_blocks是一个连接起所有未分配块的链表
*/


static inline ssize_t alloc_pages_tool( size_t page_start, size_t page_end );
static inline ssize_t alloc_pages_tool1( size_t page_start, size_t page_limit );
static inline void free_pages_tool( size_t page_start, size_t page_end );
static inline void free_pages_tool1( size_t page_start, size_t page_limit );

void *fake_malloc(const size_t size)
{
    // >= 2^48
    if ( REMOVE_BITS_LOW(size, 48) !=0 )
    {
        return NULL;
    }
    if ( size == 0 )
    {
        return (void *)((size_t)NULL+1);
    }
    // 先遍历一遍空闲块，看是否有合适的
    {
        Block *free_block;
        list_for_each_entry(free_block, &head_free_blocks, header.node_free_blocks)
        {



            if ( free_block->header.flag != 0 )
            {
                printf("malloc error!\n");
                exit(1);
            }



            if ( free_block->header.size<size )
            {
                continue;
            }
            size_t const next_block_t=(size_t)&free_block->content[free_block->header.size];
            if ( free_block->header.size <= size+sizeof(Block_Header) )
            {
                // free aera : free_block->content[ 0 to (free_block->header.size-1) ]
                if (
                        alloc_pages_tool(
                            REMOVE_BITS_LOW( (size_t)free_block->content-1 , 21) + (((size_t)1)<<21),
                            REMOVE_BITS_LOW( next_block_t , 21)
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
                Block *const new_block=(Block*)(new_start_t-sizeof(Block_Header));
                {
                    size_t temp_page_start=REMOVE_BITS_LOW((size_t)new_block, 21);
                    if ( temp_page_start == REMOVE_BITS_LOW((size_t)free_block->content-1, 21) )
                    {
                        temp_page_start+=((size_t)1)<<21;
                    }
                    if ( alloc_pages_tool(temp_page_start, REMOVE_BITS_LOW(next_block_t, 21)) != 0 )
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
        if ( alloc_pages(_BASE, ((size+sizeof(Block_Header)-1)>>21) +1 ) != 0 )
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
                    REMOVE_BITS_LOW((size_t)new_block-1, 21)+(((size_t)1)<<21),
                    REMOVE_BITS_LOW((size_t)&new_block->content[size]-1, 21)
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

void fake_free(void *base)
{
    Block *const del_block=(Block *)((size_t)base-sizeof(Block_Header));


    if ( del_block->header.flag != 1 )
    {
        printf("free invalid address!\n");
        exit(1);
    }


    if ( del_block == last_block )
    {
        if ( del_block == _BASE )
        {
            // 没有更多的块了，清空堆
            free_pages(_BASE, ( (del_block->header.size+sizeof(Block_Header)-1) >> 21)+1);
            last_block=NULL;
            return;
        }
        Block *const prev_block=del_block->header.prev;
        if ( prev_block->header.flag == 0 )
        {
            if ( prev_block == _BASE)
            {
                // 没有更多的块了，清空堆
                free_pages(_BASE, ( ((size_t)&del_block->content[del_block->header.size-1]-(size_t)_BASE) >> 21)+1);
                head_free_blocks.prev=head_free_blocks.next=&head_free_blocks;
                last_block=NULL;
                return;
            }
            // 删除 del 块，prev 块
            // free del prev.header
            list_del(&prev_block->header.node_free_blocks);
            last_block=prev_block->header.prev;
            size_t free_page_start=REMOVE_BITS_LOW((size_t)del_block, 21);
            size_t const prev_header_page=REMOVE_BITS_LOW((size_t)prev_block->content-1, 21);
            if ( GET_BITS_LOW((size_t)prev_block->content-1, 21) < sizeof(Block_Header) )
            {
                if ( free_page_start - prev_header_page <= (((size_t)1)<<21) )
                {
                    free_pages_tool1(prev_header_page, REMOVE_BITS_LOW((size_t)&del_block->content[del_block->header.size-1], 21));
                }
                else
                {
                    free_pages((void *)prev_header_page, 1);
                    free_pages_tool1(free_page_start, REMOVE_BITS_LOW((size_t)&del_block->content[del_block->header.size-1], 21));
                }
            }
            else
            {
                if ( free_page_start == prev_header_page )
                {
                    free_page_start+=((size_t)1)<<21;
                }
                free_pages_tool1(free_page_start, REMOVE_BITS_LOW((size_t)&del_block->content[del_block->header.size-1], 21));
            }
        }
        else
        {
            // 删除del 块
            // free del
            last_block=prev_block;
            free_pages_tool1(REMOVE_BITS_LOW((size_t)del_block-1, 21)+ (((size_t)1)<<21), REMOVE_BITS_LOW((size_t)&del_block->content[del_block->header.size-1], 21));
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
            size_t free_page_limit=REMOVE_BITS_LOW((size_t)next_block->content-1, 21);
            if ( free_page_limit == REMOVE_BITS_LOW(next_next_block_t, 21) )
            {
                free_pages_tool((size_t)_BASE+(((size_t)1)<<21), free_page_limit);
            }
            else
            {
                free_pages_tool1((size_t)_BASE+(((size_t)1)<<21), free_page_limit);
            }
        }
        else
        {
            // free del.content
            list_add(&del_block->header.node_free_blocks, &head_free_blocks);
            free_pages_tool((size_t)_BASE+(((size_t)1)<<21), REMOVE_BITS_LOW((size_t)next_block, 21) );
        }
        return;
    }

    Block *const prev_block=del_block->header.prev;
    if ( prev_block->header.flag == 0 )
    {
        size_t free_page_start=REMOVE_BITS_LOW((size_t)del_block, 21);
        if ( free_page_start == REMOVE_BITS_LOW((size_t)prev_block->content-1, 21) )
        {
            free_page_start+=((size_t)1)<<21;
        }
        if ( next_block->header.flag == 0 )
        {
            // 删除 del 块和next块
            // free del + next.header
            size_t const next_next_block_t=(size_t)&next_block->content[next_block->header.size];
            prev_block->header.size=next_next_block_t-(size_t)prev_block->content;
            list_del(&next_block->header.node_free_blocks);
            size_t const free_page_limit=REMOVE_BITS_LOW((size_t)next_block->content-1, 21);
            if ( free_page_limit == REMOVE_BITS_LOW(next_next_block_t, 21) )
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
            free_pages_tool(free_page_start, REMOVE_BITS_LOW((size_t)next_block, 21));
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
            size_t const free_pages_limit=REMOVE_BITS_LOW((size_t)next_block->content-1, 21);
            if ( free_pages_limit == REMOVE_BITS_LOW(next_next_block_t, 21) )
            {
                free_pages_tool(REMOVE_BITS_LOW((size_t)del_block->content-1, 21)+(((size_t)1)<<21), free_pages_limit);
            }
            else
            {
                free_pages_tool1(REMOVE_BITS_LOW((size_t)del_block->content-1, 21)+(((size_t)1)<<21), free_pages_limit);
            }
        }
        else
        {
            // free del.content
            list_add(&del_block->header.node_free_blocks, &head_free_blocks);
            free_pages_tool(
                    REMOVE_BITS_LOW((size_t)del_block->content-1, 21)+(((size_t)1)<<21),
                    REMOVE_BITS_LOW((size_t)next_block, 21)
                    );
        }
    }
}

inline ssize_t alloc_pages_tool( size_t page_start, size_t page_end )
{
    if ( page_end > page_start )
    {
        return alloc_pages((void *)page_start, (page_end-page_start)>>21 );
    }
    return 0;
}

inline ssize_t alloc_pages_tool1( size_t page_start, size_t page_limit )
{
    if ( page_limit >= page_start )
    {
        return alloc_pages((void *)page_start, ((page_limit-page_start)>>21)+1 );
    }
    return 0;
}

inline void free_pages_tool( size_t page_start, size_t page_end )
{
    if ( page_end > page_start )
    {
        free_pages((void *)page_start, (page_end-page_start)>>21 );
    }
}

inline void free_pages_tool1( size_t page_start, size_t page_limit )
{
    if ( page_limit >= page_start )
    {
        free_pages((void *)page_start, ((page_limit-page_start)>>21)+1 );
    }
}
