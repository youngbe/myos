#pragma once
#include <list.h>
#include <nhlist.h>
#include <tsl_lock.h>
#include <stdint.h>
#include <stddef.h>
#include <tools.h>

#define __CS        (1<<3)
#define __DS        (2<<3)
#define __DS_USER   ((3<<3)|0b11)
#define __CS_USER   ((4<<3)|0b11)
#define __TSS       (5<<3)

// 内核栈(每个线程一个)的大小
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

#ifndef IN_KERNEL_INIT
extern size_t const cores_num;
extern struct TSS64 *const tsss;
// 内存相关
// 页表
extern uint64_t kernel_pt1s[64][512];
extern const uint64_t halt_pt0[512];
extern uint64_t (*const page_tables)[512];
extern uint_fast16_t *const pte_nums;
extern uint64_t (**const free_page_tables)[512];
extern size_t free_page_tables_num;
// 所有空闲页
extern uint64_t *const free_pages;
extern size_t free_pages_num;
// 对 free_pages, free_pages_num, free_page_tables, free_page_tables_num 加锁
//extern Semaphore mm_mutex;

// 调度相关
// 可调度线程表
extern struct list_nohead *index_sched_threads;
extern tsl_mutex sched_threads_mutex;
// 当前cpu正在运行的线程
// running_threads[0] == 0号CPU核心正在运行的线程
// running_threads[1] == 1号CPU核心正在运行的线程
extern Thread **const running_threads;
// CPU挂起时使用的栈
extern struct __attribute__((aligned(16))){uint8_t padding[HALT_STACK_SIZE];}
*const halt_stacks;
#endif

static inline size_t get_coreid()
{
    uint32_t core_id;
    __asm__ volatile(
            "rdmsr"
            :"=a"(core_id)
            :"c"((uint32_t)0x802)
            :"edx"
            );
    return (size_t)core_id;
}

__attribute__((noreturn)) static inline void kernel_abort(const char * str)
{
    struct __attribute__((packed)) Temp
    {
        uint16_t x[80*25];
    };
    typedef struct Temp Temp;
#define REPEAT_M(_, __, ___) 0x0700,
    *(Temp *)0xb8000=(Temp){{BOOST_PP_REPEAT(2000, REPEAT_M, _)}};
#undef REPEAT_M
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

static inline void  wrmsr(uint32_t msr, uint64_t val)
{
    __asm__ volatile(
            "wrmsr"
            :
            :"a"((uint32_t)val), "d"((uint32_t)(val>>32)), "c"(msr)
            :);
}

static inline uint64_t rdmsr(uint32_t msr)
{
    uint32_t low;
    uint32_t high;
    __asm__ volatile(
            "rdmsr"
            :"=a"(low), "=d"(high)
            :"c"(msr)
            :);
    return ((uint64_t)high<<32)|low;
}

static inline uint64_t (*get_cr3())[512]
{
    uint64_t (*cr3)[512];
    __asm__ volatile(
            "movq   %%cr3, %0"
            :"=r"(cr3)
            :
            :);
    return cr3;
}

static inline void write_eoi()
{
    __asm__ volatile(
            "wrmsr"
            :
            :"c"((uint32_t)0x80b), "a"((uint32_t)0), "d"((uint32_t)0)
            :);
}

extern void return_handler_thread_start();
extern void return_handler_int();
extern void return_handler_function();
