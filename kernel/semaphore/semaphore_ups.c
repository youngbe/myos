#include "semaphore.h"

void semaphore_ups(Semaphore *const sema, size_t val)
{
    if ( val == 0 )
    {
        return;
    }
    CLI_TSL_LOCK_CONTENT(sema->mutex, "=m"(*sema));
    if (  list_empty(&sema->head_block_threads) )
    {
        sema->val+=val;
        STI_TSL_UNLOCK_CONTENT(sema->mutex, "m"(*sema));
        return;
    }
    struct list_nohead *new_sched_node;
    {
        struct list_head *const temp_node=sema->head_block_threads.next;
        __list_del(&sema->head_block_threads, temp_node->next);
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
    --val;
    while ( val != 0 && !list_empty(&sema->head_block_threads) )
    {
        struct list_head *const temp_node=sema->head_block_threads.next;
        __list_del(&sema->head_block_threads, temp_node->next);
        new_sched_node=&list_entry(temp_node, Thread, node_block_threads)->node_sched_threads;
        nhlist_add(new_sched_node, index_sched_threads);
        --val;
    }
    TSL_UNLOCK(sched_threads_mutex);
    sema->val+=val;
    STI_TSL_UNLOCK_CONTENT(sema->mutex, "m"(*sema));
}
