#include "public.h"

#include "switch.h"

// 时钟中断
// 如果完全使用汇编语言编写，工程量太大
// 如果完全使用C语言编写，难以告诉编译器希望的行为。特别是栈和线程切换的处理
// 因此使用C编写，编译为汇编文件后再修改

//此文件不应该被编译，请使用timer_isr.s;

__attribute__ ((interrupt))
void timer_isr(const void *)
{
    size_t const core_id=get_coreid();
    Thread *const running_thread=running_threads[core_id];
    TSL_LOCK_CONTENT(sched_threads_mutex, "=m"(index_sched_threads));
    write_eoi();
    struct list_nohead *const new_node=index_sched_threads;
    if ( new_node == NULL )
    {
        TSL_UNLOCK_CONTENT(sched_threads_mutex, "m"(index_sched_threads));
        return;
    }
    struct list_nohead *const next_node=new_node->next;
    Thread *const new_thread=nhlist_entry(new_node, Thread, node_sched_threads );
    if ( running_thread == NULL )
    {
        if ( new_node == next_node )
        {
            index_sched_threads=NULL;
        }
        else
        {
            nhlist_del(new_node);
            index_sched_threads=next_node;
        }
        __asm__ volatile(
                "movq   %[new_cr3], %%cr3"
                :
                :[new_cr3]"r"(new_thread->cr3)
                :);
    }
    else
    {
        // 保存上下文，其中%rdi %rsi 等编译器已经保存
        __asm__ volatile(
                //"pushq  %%rdi\n\t"
                //"pushq  %%rsi\n\t"
                //"pushq  %%rbx\n\t"
                //"pushq  %%rcx\n\t"
                //"pushq  %%rdx\n\t"
                //"pushq  %%rax\n\t"
                //"pushq  %%rbp\n\t"
                "pushq  %%r12\n\t"
                "pushq  %%r13\n\t"
                "pushq  %%r14\n\t"
                "pushq  %%r15\n\t"
                "pushq  %%r8\n\t"
                "pushq  %%r9\n\t"
                "pushq  %%r10\n\t"
                "pushq  %%r11\n\t"
                "movq   %%rsp, %[rsp]"
                :[rsp]"=m"(running_thread->rsp)
                :
                :
                );
        running_thread->return_handler=return_handler_timer_int;
        if ( new_node == next_node )
        {
            running_thread->node_sched_threads.next=running_thread->node_sched_threads.prev=index_sched_threads=&running_thread->node_sched_threads;
        }
        else
        {
            nhlist_replace(new_node, &running_thread->node_sched_threads);
            index_sched_threads=next_node;
        }
        {
            uint64_t temp;
            __asm__ volatile(
                    "movq   %%cr3, %[temp]\n\t"
                    "cmpq   %[temp], %[new_cr3]\n\t"
                    "je     1f\n\t"
                    "movq   %[new_cr3], %%cr3\n"
                    "1:"
                    :[temp]"=&r"(temp)
                    :[new_cr3]"r"(new_thread->cr3)
                    :"cc");
        }
    }
    TSL_UNLOCK_CONTENT(sched_threads_mutex, "m"(index_sched_threads));
    switch_to_thread(new_thread, core_id);
}
