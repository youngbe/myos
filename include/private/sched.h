#include <list.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_THREADS_NUM 1024
#define THREAD_STACK_SIZE 1024
#define CPU_STACK_SIZE 1024

typedef struct Thread Thread;
struct Thread
{
    uint8_t thread_stack[THREAD_STACK_SIZE];
    List schedulable_thread_list;
};

extern Thread thread_pool[MAX_THREADS_NUM];
extern size_t schedulable_thread_list_mutex;
extern List* schedulable_thread_list_index;
extern Thread** running_threads;
extern uint8_t (*cpu_stacks)[CPU_STACK_SIZE];
