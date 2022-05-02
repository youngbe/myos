#include "semaphore.h"

void semaphore_up(Semaphore *const sema)
{
    __asm__ volatile(
            "cli"
            :"=m"(sema->mutex)
            :
            :);
    TSL_LOCK_CONTENT(sema->mutex, "=m"(*sema));
    if (  list_empty(&sema->head_block_threads) )
    {
        ++sema->val;
        TSL_UNLOCK_CONTENT(sema->mutex, "m"(*sema));
        __asm__ volatile(
                "sti"
                :
                :"m"(sema->mutex)
                :);
        return;
    }
    struct list_nohead *new_sched_node;
    {
        struct list_head *const temp_node=sema->head_block_threads.next;
        __list_del(&sema->head_block_threads, temp_node->next);
        TSL_UNLOCK_CONTENT(sema->mutex, "m"(*sema));
        new_sched_node=&list_entry(temp_node, Thread, node_block_threads)->node_sched_threads;
    }
    TSL_LOCK_CONTENT(sched_threads_mutex, "=m"(index_sched_threads));
    struct list_nohead *const temp_index=index_sched_threads;
    if ( temp_index == NULL )
    {
        init_nhlist(&index_sched_threads, new_sched_node);
    }
    else
    {
        nhlist_add(new_sched_node, index_sched_threads);
    }
    TSL_UNLOCK(sched_threads_mutex);
    __asm__ volatile(
            "sti"
            :
            :"m"(sched_threads_mutex)
            :);
}
