#include <stddef.h>
#include <stdint.h>

#include <bit.h>

#include <tsl_lock.h>

#include "public.h"
#include "semaphore/semaphore.h"

#if UINTPTR_MAX != UINT64_MAX
只支持 x86_64指令集
#endif

// 内核虚拟空间 0-32T
// KERNEL_BASE_PAGE：内核虚拟空间的起始页
// KERNEL_LIMIT_PAGE：内核虚拟空间的最后一页
#define KERNEL_BASE_PAGE ((uint64_t)1<<21)
#define KERNEL_LIMIT_PAGE (((uint64_t)1<<45)-((uint64_t)1<<21))

// 用户虚拟空间 32T-256T
#define USER_BASE_PAGE ((uint64_t)1<<45)
#define USER_LIMIT_PAGE (((uint64_t)1<<48)-((uint64_t)1<<21))


static inline size_t calc_max_need_page_tables_num(uint64_t const base, uint64_t const limit);
// 不会检查剩余空闲页表是否足够
// 如果已经有映射，则报错
static inline void map_page_2m_kernel(const uint64_t v_page, const uint64_t phy_page);
//static inline void map_page_2m_user(const uint64_t v_page, const uint64_t phy_page, uint64_t (*const cr3)[512]);
static inline uint64_t unmap_page_2m_kernel(const uint64_t v_page);
static inline uint_fast16_t * get_pte_num(uint64_t (*)[512]);


int alloc_pages_kernel(void *const base, const size_t num)
{
    uint64_t base_page=(uint64_t)base;
    if ( GET_BITS_LOW(base_page, 21) != 0 )
    {
        return -1;
    }
    if ( num == 0 )
    {
        return 0;
    }
    if ( base_page < KERNEL_BASE_PAGE || base_page > KERNEL_LIMIT_PAGE )
    {
        return -1;
    }
    if ( ((KERNEL_LIMIT_PAGE - base_page)>>21)+1 < num )
    {
        return -1;
    }
    uint64_t const limit_page=base_page+((uint64_t)(num-1)<<21);
    size_t const max_need_page_tables_num=calc_max_need_page_tables_num(base_page, limit_page);
    semaphore_down(&mm_mutex);
    if ( num>free_pages_num || max_need_page_tables_num > free_page_tables_num )
    {
        semaphore_up(&mm_mutex);
        return -1;
    }
    while ( true )
    {
        map_page_2m_kernel(base_page, free_pages[--free_pages_num]);
        if ( base_page == limit_page )
        {
            break;
        }
        else
        {
            base_page+=(uint64_t)1<<21;
        }
    }
    semaphore_up(&mm_mutex);
    return 0;
}
void free_pages_kernel(void *const base, const size_t num)
{
    if ( num == 0 )
    {
        return;
    }
    uint64_t base_page=(uint64_t)base;
    uint64_t const limit_page=base_page+((uint64_t)(num-1)<<21);
    semaphore_down(&mm_mutex);
    while ( true )
    {
        free_pages[free_pages_num++]=unmap_page_2m_kernel(base_page);
        if ( base_page == limit_page )
        {
            break;
        }
        else
        {
            base_page+=(uint64_t)1<<21;
        }
    }
    semaphore_up(&mm_mutex);
}

inline uint64_t unmap_page_2m_kernel(const uint64_t v_page)
{
    uint64_t const i0=v_page>>39;
    // i0一定小于64
    uint64_t (*const pt1)[512]=&kernel_pt1s[i0];
    if ( (uint64_t)pt1 == 0 )
    {
        kernel_abort("kernel free not exist page!");
    }

    const uint64_t i1=GET_BITS_LOW(v_page>>30, 9);
    uint64_t (*const pt2)[512]=(uint64_t (*)[512])REMOVE_BITS_LOW((*pt1)[i1], 12);
    if ( (uint64_t)pt2 == 0 )
    {
        kernel_abort("kernel free not exist page!");
    }

    uint64_t const i2=GET_BITS_LOW(v_page>>21, 9);
    uint64_t const phy_page=REMOVE_BITS_LOW((*pt2)[i2], 21);
    if ( (uint64_t)phy_page == 0 )
    {
        kernel_abort("kernel free not exist page!");
    }
    uint_fast16_t *const pt2_pte_num=get_pte_num(pt2);

    if ( --*pt2_pte_num == 0 )
    {
        free_page_tables[free_page_tables_num++]=pt2;
        (*pt1)[i1]=0;
    }
    else
    {
        (*pt2)[i2]=0;
    }
    return phy_page;
}

inline void map_page_2m_kernel(const uint64_t v_page, const uint64_t phy_page)
{
    // cr3(pt0) -> 0级页表.pt1 -> 1级页表.pt2 -> 2级页表 -> 2mb物理页
    uint64_t i=v_page>>39;
    // i一定小于64
    uint64_t (*const pt1)[512]=&kernel_pt1s[i];
    i=GET_BITS_LOW(v_page>>30, 9);
    uint64_t (*pt2)[512];
    if ( (*pt1)[i] == 0 )
    {
        pt2=free_page_tables[--free_page_tables_num];
        (*pt1)[i]=(uint64_t)pt2|((uint64_t)1<<0);
    }
    else
    {
        pt2=(uint64_t (*)[512])REMOVE_BITS_LOW((*pt1)[i], 12);
    }
    i=GET_BITS_LOW(v_page>>21, 9);
    if ( (*pt2)[i] != 0 )
    {
        kernel_abort("kernel map exist page!");
    }
    (*pt2)[i]=phy_page|((uint64_t)1<<0)|((uint64_t)1<<7);
    ++*get_pte_num(pt2);
}

inline uint_fast16_t * get_pte_num(uint64_t (*pt)[512])
{
    return &pte_nums[pt-page_tables];
}

inline size_t calc_max_need_page_tables_num(uint64_t const base, uint64_t const limit)
{
    // 计算覆盖多少512G页
    size_t ret=(limit>>39)-(base>>39)+1;
    // 计算覆盖多少1G页
    ret+=(limit>>30)-(base>>30)+1;
    return ret;
}
