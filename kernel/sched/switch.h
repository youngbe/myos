#pragma once

#include "public.h"
#include "init_output.h"

extern void return_handler_timer_int();
extern void return_handler_function();
extern void return_handler_thread_start();

// 进入前必须关闭中断
__attribute__((noreturn))
static inline void switch_to_thread(Thread *const new_thread, size_t const core_id)
{
    running_threads[core_id]=new_thread;
    tsss[core_id].rsp0=(uint64_t)&new_thread->kernel_stack_end;
    __asm__ volatile(
            "movq   %[rsp], %%rsp\n\t"
            "jmpq   *%[return_handler]"
            :
            :[rsp]"g"(new_thread->rsp), [return_handler]"rm"(new_thread->return_handler)
            :"memory"
            );
    __builtin_unreachable();
}

__attribute__((noreturn))
static inline void switch_to_halt( size_t const core_id)
{
    running_threads[core_id]=NULL;
    __asm__ volatile(
            "movq   %[rsp], %%rsp\n\t"
            "sti\n"
            "1:\n\t"
            "hlt\n\t"
            "jmp    1b"
            :
            :[rsp]"g"(&halt_stacks[core_id+1])
            :"memory");
    __builtin_unreachable();
}
