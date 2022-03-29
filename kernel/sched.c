#include <sched.h>
#include <coreid.h>

#include <system_table.h>

Thread thread_pool[MAX_THREADS_NUM];
size_t schedulable_thread_list_mutex=0;
List* schedulable_thread_list_index=NULL;

static Thread* running_thread0=NULL;
Thread** running_threads=&running_thread0;

static uint8_t __attribute__((aligned(32))) cpu0_stack[CPU_STACK_SIZE];
uint8_t (*cpu_stacks)[CPU_STACK_SIZE]=&cpu0_stack;

// return new timer stack
void* timer_isr_helper()
{
    const size_t coreid=get_coreid();
    Thread*const running_thread=running_threads[coreid];
    List*const last=schedulable_thread_list_index;
    List*const current=last->next;
    if ( running_thread == NULL )
    {
        if ( last == current )
        {
            schedulable_thread_list_index=NULL;
        }
        else
        {
            last->next=current->next;
        }
    }
    else
    {
        List*const running=&running_thread->schedulable_thread_list;
        if ( last == current )
        {
            running->next=running;
        }
        else
        {
            running->next=current->next;
            last->next=running;
        }
        schedulable_thread_list_index=running;
    }
    Thread*const new_running_thread=list_entry(current, Thread, schedulable_thread_list);
    running_threads[coreid]=new_running_thread;
    tsss[coreid].ist1=(uint64_t)(size_t)new_running_thread->thread_stack+sizeof(((Thread*)0)->thread_stack);
    return new_running_thread->thread_stack+sizeof(((Thread*)0)->thread_stack)-15*8;
}

/*
volatile lock free_thread_stack_lock;
size_t free_thread_stack[1024];
size_t free_thread_stack_size=1024;

void init()
{
    for (size_t i=0; i<free_thread_stack_size; ++i)
    {
        free_thread_stack[0]=i;
    }
}



static inline struct
{
    ssize_t status;
    size_t tid;
} get_new_tid()
{
    lock(&thread_stack_lock);
    size_t temp=(volatile size_t)thread_stack_size;
    unlock(&thread_stack_lock);
    return ret;
}

void init_sched()
{
    size_t first_thread;
    {
        __auto_type ret=get_new_tid();
        if ( ret.status != 0 )
        {
            error();
        }
        first_thread=get_new_tid();
    }

}

void idt32()
{
*/
