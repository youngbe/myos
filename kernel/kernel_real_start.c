#include "public.h"
#include "mm/mm.h"
#include <tsl_lock.h>
#include "semaphore/semaphore.h"

struct list_nohead *index_sched_threads=NULL;
tsl_mutex sched_threads_mutex=TSL_UNLOCKED;
Semaphore mm_mutex=SEMAPHORE_INIT(mm_mutex, 1);

// 在初始化阶段创建进程
int create_process(const void *const start)
{
    Thread *const new_thread=kmalloc_p2align(sizeof(Thread), 4);
    if ( new_thread == NULL )
    {
        return -1;
    }
    Process *const new_process=kmalloc(sizeof(Process));
    if ( new_process == NULL )
    {
        kfree(new_thread);
        return -1;
    }
    if (free_pages_num<4 || free_page_tables_num <2)
    {
        kfree(new_thread);
        kfree(new_process);
        return -1;
    }
    semaphore_down(&mm_mutex);
    const uint64_t new_pages[4]={free_pages[free_pages_num-1], free_pages[free_pages_num-2],
    free_pages[free_pages_num-3], free_pages[free_pages_num-4]};
    free_pages_num-=4;
    uint64_t (*const new_page_tables[2])[512]={free_page_tables[free_page_tables_num-1], free_page_tables[free_page_tables_num-2]};
    free_page_tables_num-=2;
    semaphore_up(&mm_mutex);
    for ( size_t i=0; i<64; ++i )
    {
        (*new_page_tables[0])[i]=((uint64_t)&kernel_pt1s[i])|0b111;
    }
    (*new_page_tables[0])[511]=((uint64_t)new_page_tables[1])|0b111;
    (*new_page_tables[1])[511]=new_pages[3]|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2)|((uint64_t)1<<7);
    (*new_page_tables[1])[510]=new_pages[2]|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2)|((uint64_t)1<<7);
    (*new_page_tables[1])[509]=new_pages[1]|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2)|((uint64_t)1<<7);
    (*new_page_tables[1])[508]=new_pages[0]|((uint64_t)1<<0)|((uint64_t)1<<1)|((uint64_t)1<<2)|((uint64_t)1<<7);
    pte_nums[new_page_tables[0]-page_tables]=65;
    pte_nums[new_page_tables[1]-page_tables]=4;
    new_process->cr3=new_page_tables[0];
    INIT_LIST_HEAD(&new_process->head_active_threads);
    list_add(&new_thread->node_active_threads, &new_process->head_active_threads);
    INIT_LIST_HEAD(&new_process->head_dead_threads);
    new_thread->cr3=new_page_tables[0];
    new_thread->master=new_process;
    new_thread->rsp=(uint64_t)&new_thread->kernel_stack_end-32;
    // %rsi
    ((uint64_t *)new_thread->rsp)[0]=1;
    // %rdi
    ((uint64_t *)new_thread->rsp)[1]=2;
    // %rip
    ((uint64_t *)new_thread->rsp)[2]=(uint64_t)start;
    // %rsp
    ((uint64_t *)new_thread->rsp)[3]=((uint64_t)1<<48)-8;
    new_thread->return_handler=return_handler_thread_start;
    CLI_TSL_LOCK_CONTENT(sched_threads_mutex, "=m"(index_sched_threads));
    if ( index_sched_threads == NULL )
    {
        init_nhlist(&index_sched_threads, &new_thread->node_sched_threads);
    }
    else
    {
        nhlist_add(&new_thread->node_sched_threads, index_sched_threads);
    }
    STI_TSL_UNLOCK(sched_threads_mutex);
    return 0;
}

void thread1();
void thread2();

void kernel_real_start()
{
    if ( create_process((void*)thread1) != 0 )
    {
        kernel_abort("init process failed!");
    }
    if ( create_process((void*)thread2) != 0 )
    {
        kernel_abort("init process failed!");
    }
    __asm__ volatile(
            "1:hlt\n\t"
            "jmp    1b"
            :
            :
            :"memory");
    __builtin_unreachable();
}
void thread1()
{
    static size_t i=0;
    while (1)
    {
        (*(char (*)[80*25*2])0xb8000)[i]='x';
        i+=2;
        for ( volatile size_t i=0; i<(1<<25); ++i )
        {
        }
    }
}

void thread2()
{
    static size_t i=80*25*2-2;
    while (1)
    {
        (*(char (*)[80*25*2])0xb8000)[i]='y';
        i-=2;
        for ( volatile size_t i=0; i<(1<<25); ++i )
        {
        }
    }
}
