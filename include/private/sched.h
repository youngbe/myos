#include <list.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_THREADS_NUM 1024
#define THREAD_STACK_SIZE 1024

typedef struct Thread Thread;
struct Thread
{
    // 每个线程有一个thread_stack，用于处理正在运行该线程时发生的时钟中断
    // 仅处理时钟中断，其他中断在core_stack上处理
    uint8_t thread_stack[THREAD_STACK_SIZE];
    List schedulable_thread_list;
};

extern Thread thread_pool[MAX_THREADS_NUM];
extern size_t schedulable_thread_list_mutex;
extern List* schedulable_thread_list_index;
// 每个逻辑核心一个Thread*，代表每个逻辑核心上正在运行的线程
extern Thread** running_threads;
