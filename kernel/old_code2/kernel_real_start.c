#include <stddef.h>
#include <stdint.h>

#include <nhlist.h>
#include <tsl_lock.h>
#include <bit.h>

#include "public.h"
#include "init_output.h"
#include "mm.h"



struct list_nohead* index_sched_threads=NULL;

static inline void new_thread(void * start)
{
    struct Thread *const new_thread=(struct Thread*)kmalloc_p2align(sizeof(struct Thread), 4);
    if ( new_thread == NULL )
    {
        kernel_abort("Memory not enougth!");
    }
    new_thread->rsp=(void*)(new_thread->kernel_stack_end-20*8);
    // %rip
    ((uint64_t *)new_thread->rsp)[15]=(uint64_t)start;
    // %cs
    ((uint64_t *)new_thread->rsp)[16]=__CS;
    // %eflags
    ((uint64_t *)new_thread->rsp)[17]=((uint64_t)1<<9)|((uint64_t)1<<1);
    // %rsp
    ((uint64_t *)new_thread->rsp)[18]=(uint64_t)new_thread->kernel_stack_end;
    // %ss
    ((uint64_t *)new_thread->rsp)[19]=__DS;
    if ( index_sched_threads == NULL )
    {
        index_sched_threads=&new_thread->node_sched_threads;
        new_thread->node_sched_threads.next=new_thread->node_sched_threads.prev=&new_thread->node_sched_threads;
    }
    else
    {
        nhlist_add(&new_thread->node_sched_threads, index_sched_threads);
    }
}

void thread1();
void thread2();

__attribute__((noreturn))
void kernel_real_start()
{
    //void *const thread1_stack=kmalloc(USER_STACK_SIZE);
    //void *const thread2_stack=kmalloc(USER_STACK_SIZE);
    new_thread((void*)thread1);
    //, (void*)(REMOVE_BITS_LOW((uint64_t)thread1_stack+USER_STACK_SIZE+8, 4)-8) );
    new_thread((void*)thread2);
    __asm__ volatile(
            "sti\n"
            "1:\n\t"
            "hlt\n\t"
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

// return new timer stack
void* timer_isr_helper(void *const rsp)
{
    if ( index_sched_threads == NULL )
    {
        __builtin_unreachable();
    }
    size_t const core_id=get_coreid();
    Thread *const running=running_threads[core_id];
    struct list_nohead *const current=index_sched_threads;
    struct list_nohead *const next=current->next;
    if ( running == NULL )
    {
        if ( current == next )
        {
            index_sched_threads=NULL;
        }
        else
        {
            nhlist_del(current);
            index_sched_threads=next;
        }
    }
    else
    {
        running->rsp=rsp;
        if ( current == next )
        {
            running->node_sched_threads.next=running->node_sched_threads.prev=index_sched_threads=&running->node_sched_threads;
        }
        else
        {
            nhlist_replace(current, &running->node_sched_threads);
            index_sched_threads=next;
        }
    }
    Thread *const new_running=running_threads[core_id]=nhlist_entry(current, Thread, node_sched_threads);
    tsss[core_id].rsp0=(uint64_t)&new_running->kernel_stack_end;
    return new_running->rsp;
}
