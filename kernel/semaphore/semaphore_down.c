#include "semaphore.h"

void semaphore_down(Semaphore *const sema)
{
    CLI_TSL_LOCK_CONTENT(sema->mutex, "=m"(*sema));
    if ( sema->val != 0 )
    {
        --sema->val;
        STI_TSL_UNLOCK_CONTENT(sema->mutex, "m"(*sema));
        return;
    }
    // 将线程阻塞到信号量上
    size_t const core_id=get_coreid();
    Thread*const running_thread=running_threads[core_id];
    running_thread->return_handler=return_handler_function;
    list_add(&sema->head_block_threads, &running_thread->node_block_threads);
    __asm__ volatile(
            "pushq  %%rbx\n\t"
            "pushq  %%r12\n\t"
            "pushq  %%r13\n\t"
            "pushq  %%r14\n\t"
            "pushq  %%r15\n\t"
            "pushq  %%rbp\n\t"
            "movq   %%rsp, %[rsp]"
            :[rsp]"=m"(running_thread->rsp)
            :
            :
            );

    // 选择新的线程运行
    // 1. 上锁可调度线程表
    // 2. 选择新线程(从线程表中取走)
    // 3. 解锁可调度线程表
    // 4. 更换cr3
    // 5. 解锁信号量
    // 6. 切换到新线程上运行
    // 必须先切换cr3再解锁信号量，因为解锁信号量后，这个线程的cr3可能失效
    TSL_LOCK_CONTENT(sched_threads_mutex, "=m"(index_sched_threads));
    struct list_nohead *const temp_index=index_sched_threads;
    // 没有可调度线程
    if ( temp_index == NULL )
    {
        TSL_UNLOCK_CONTENT(sched_threads_mutex, "m"(index_sched_threads));
        __asm__ volatile(
                "movq   %[cr3], %%cr3"
                :
                :[cr3]"r"((uint64_t)&halt_pt0)
                :"memory");
        TSL_UNLOCK_CONTENT(sema->mutex, "m"(*sema));
        switch_to_halt(core_id);
    }
    else
    {
        if ( temp_index->next == temp_index )
        {
            index_sched_threads = NULL;
        }
        else
        {
            index_sched_threads=temp_index->next;
            nhlist_del(temp_index);
        }
        TSL_UNLOCK_CONTENT(sched_threads_mutex, "m"(index_sched_threads));
        Thread*const new_running=list_entry(temp_index, Thread, node_sched_threads);
        {
            uint64_t temp;
            __asm__ volatile(
                    "movq   %%cr3, %[temp]\n\t"
                    "cmpq   %[temp], %[new_cr3]\n\t"
                    "je     1f\n\t"
                    "movq   %[new_cr3], %%cr3\n"
                    "1:"
                    :[temp]"=&r"(temp)
                    :[new_cr3]"r"(new_running->cr3)
                    :"cc", "memory");
        }
        TSL_UNLOCK_CONTENT(sema->mutex, "m"(*sema));
        switch_to_thread(new_running, core_id);
    }
}
