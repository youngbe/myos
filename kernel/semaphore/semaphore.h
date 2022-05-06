#pragma once

#include <list.h>
#include <stddef.h>
#include <tsl_lock.h>

typedef struct Semaphore Semaphore;
struct Semaphore
{
    tsl_mutex mutex;
    size_t val;
    struct list_head head_block_threads;
};

#define SEMAPHORE_INIT(m, val) {TSL_UNLOCKED, (val), LIST_HEAD_INIT((m).head_block_threads)}

static inline void init_semaphore(Semaphore*const sema, const size_t val)
{
    *sema=(Semaphore)SEMAPHORE_INIT(*sema, val);
}

void semaphore_up(Semaphore* sema);
void semaphore_ups(Semaphore* sema);
void semaphore_down(Semaphore* sema);
