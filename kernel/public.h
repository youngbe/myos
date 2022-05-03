#pragma once
#include <list.h>
#include <nhlist.h>
#include <tsl_lock.h>
#include <stdint.h>
#include <stddef.h>

#define __CS        (1<<3)
#define __CS_USER   ((2<<3)|0b11)
#define __DS        (0<<3)
#define __DS_USER   ((3<<3)|0b11)
#define __TSS       (4<<3)

// 内核栈(每个线程一个)的大小，必须对齐16字节
// 当使用系统调用时，或者发生时钟中断时，就从进程的用户态内存切换到这里
#define KERNEL_STACK_SIZE 0x10000
// 用户栈(每个线程一个)的大小，8M
#define USER_STACK_SIZE ((size_t)1<<23)
// CPU挂起时运行的栈
#define HALT_STACK_SIZE 0x1000

struct __attribute__ ((packed, aligned(32))) TSS64
{
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t io_map_base_address;
};

typedef struct Process Process;
struct Process
{
    uint64_t (*cr3)[512];
    struct list_head head_active_threads;
    struct list_head head_dead_threads;
};

typedef struct Thread Thread;
struct Thread
{
    uint8_t kernel_stack[KERNEL_STACK_SIZE];
    uint8_t kernel_stack_end[0] __attribute__((aligned(16)));
    struct list_nohead node_sched_threads;
    void *rsp;
    void (*return_handler)();
    uint64_t (*cr3)[512];
    Process *master;
    struct list_head node_active_threads;
    struct list_head node_dead_threads;
    struct list_head node_block_threads;
};

extern struct list_nohead *index_sched_threads;
extern tsl_mutex sched_threads_mutex;

static inline size_t get_coreid()
{
    uint32_t core_id;
    __asm__ volatile(
            "rdmsr"
            :"=a"(core_id)
            :"c"((uint32_t)0x803)
            :"edx"
            );
    return (size_t)core_id;
}

    __attribute__((noreturn))
static inline void kernel_abort(const char * str)
{
    struct __attribute__((packed)) Temp
    {
        uint16_t x[80*25];
    };
    *(struct Temp *)0xb8000=(struct Temp){ {[0 ... 80*25-1] = 0x0700} };
    char *v=(char *)0xb8000;
    while ( *str != '\0' )
    {
        *v=*str;
        ++str;
        v+=2;
    }
    __asm__ volatile(
            "cli\n"
            "1:\n\t"
            "hlt\n\t"
            "jmp   1b"
            :
            :"m"(*(char (*)[80*25*2])0xb8000)
            :
            );
    __builtin_unreachable();
}

#include "sched/switch.h"
