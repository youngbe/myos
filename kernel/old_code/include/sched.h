#pragma once
#include <stddef.h>
#include <stdint.h>

#include <nhlist.h>


/*
hang_up
get_coreid
get_core_nums
*/

typedef struct Thread Thread;
struct Thread
{
    // 每个线程有一个thread_stack，用于处理正在运行该线程时发生的时钟中断
    // 仅处理时钟中断，其他中断在core_stack上处理
    void *thread_stack;
    struct list_nohead node_sched_threads;
};

extern Thread *thread_pool;
extern struct list_nohead *sched_threads;
// 每个逻辑核心一个Thread*，代表每个逻辑核心上正在运行的线程
extern Thread** running_threads;

__attribute__((noreturn))
static inline void hang_up()
{
    __asm__ volatile
        (
         ".Lhang_up%=:\n\t"
         "hlt\n\t"
         "jmp   .Lhang_up%="
         :
         :
         // 让 gcc 寄存器中的值写入内存后再挂起
         :"memory"
         );
    __builtin_unreachable();
}

static inline size_t get_coreid()
{
    uint32_t apicid;
    __asm__ volatile
        (
         "rdmsr"
         :"=a"(apicid)
         :"c"((uint32_t)0x802)
         :
         );
    return (size_t)apicid;
}
