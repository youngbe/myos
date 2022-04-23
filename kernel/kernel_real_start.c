#include <stddef.h>
#include <stdint.h>

#include <nhlist.h>
#include <mm.h>
#include <kernel.h>
#include <tsl_lock.h>

#define STACK_SIZE 4096

extern uint64_t **const timer_rsp;
extern const size_t cores_num;
extern uint8_t (*const core_stacks)[CORE_STACK_SIZE];


typedef struct Thread Thread;
struct Thread
{
    uint8_t stack[STACK_SIZE];
    uint8_t stack_end[0] __attribute__((aligned(16)));
    struct list_nohead node_sched_threads;
};

static Thread *running_thread0=NULL;
Thread** running_threads=&running_thread0;
struct list_nohead* index_sched_threads=NULL;

static inline void new_thread(void * start, void *stack_top)
{
    struct Thread *const new_thread=(struct Thread*)kmalloc_p2align(sizeof(struct Thread), 4);
    if ( new_thread == NULL )
    {
        kernel_abort("Memory not enougth!");
    }
    uint64_t *const rsp=(uint64_t *)((uint8_t *)new_thread->stack_end-40);
    // %rip
    rsp[0]=(uint64_t)start;
    // %cs
    rsp[1]=__CS;
    // %eflags
    rsp[2]=((uint64_t)1<<9)|((uint64_t)1<<1);
    // %rsp
    rsp[3]=(uint64_t)stack_top;
    // %ss
    rsp[4]=__DS;
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
    void *const thread1_stack=kmalloc_p2align(4096, 4);
    void *const thread2_stack=kmalloc_p2align(4096, 4);
    if ( thread1_stack == NULL || thread2_stack == NULL )
    {
        kernel_abort("Memory not enougth!");
    }
    new_thread((void*)thread1, (void*)((uint64_t)thread1_stack+4096));
    new_thread((void*)thread2, (void*)((uint64_t)thread2_stack+4096));
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
        for ( volatile size_t i=0; i<(1<<30); ++i )
        {
        }
    }
}

void thread2()
{
    static size_t i=80*25-2;
    while (1)
    {
        (*(char (*)[80*25*2])0xb8000)[i]='y';
        i-=2;
        for ( volatile size_t i=0; i<(1<<30); ++i )
        {
        }
    }
}

static tsl_mutex sched_threads_mutex=TSL_UNLOCKED;

// return new timer stack
void* timer_isr_helper()
{
    size_t const core_id=get_coreid();
    Thread *const running=running_threads[core_id];
    TSL_LOCK(sched_threads_mutex);
    struct list_nohead *const current=index_sched_threads;
    if ( current == NULL )
    {
        sched_threads_mutex=TSL_UNLOCKED;
        return NULL;
    }
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
        if ( current == next )
        {
            index_sched_threads=&running->node_sched_threads;
            running->node_sched_threads.next=running->node_sched_threads.prev=&running->node_sched_threads;
        }
        else
        {
            nhlist_replace(current, &running->node_sched_threads);
            index_sched_threads=next;
        }
    }
    TSL_UNLOCK(sched_threads_mutex);
    Thread *const new_running=running_threads[core_id]=nhlist_entry(current, Thread, node_sched_threads);
    *timer_rsp[core_id]=(uint64_t)&new_running->stack_end;
    return (void*)((const uint8_t *)new_running->stack_end-5*8-15*8);
}
