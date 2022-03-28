#include <list.h>
#include <stddef.h>
#include <stdint.h>
#include <sched.h>

// return new timer stack
void* timmer_isr_helper()
{
    const size_t cpuid=get_cpuid();
    List*const running=running_thread[cpuid];
    List*const last=schedulable_thread_list_index;
    List*const current=last->next;
    if ( running == NULL )
    {
        if ( last == current )
        {
            schedulable_thread_list_index=NULL;
        }
        else
        {
            last->next=current->next;
        }
        running_thread[cpuid]=current;
    }
    else
    {
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
        running_thread[cpuid]=current;
    }
    return list_entry(current, Thread, schedulable_thread_list)->timer_stack;
}
