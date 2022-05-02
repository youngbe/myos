#pragma once

#include "public.h"
#include "init_output.h"


// 进入前必须关闭中断
__attribute__((noreturn))
static inline void switch_to_thread(Thread *const new_thread)
{
    if ( new_thread->return_type == TIMER_INT )
    {
        __asm__ volatile(
                "movq   %[rsp], %%rsp\n\t"
                "popq   %%r11\n\t"
                "popq   %%r10\n\t"
                "popq   %%r9\n\t"
                "popq   %%r8\n\t"
                "popq   %%r15\n\t"
                "popq   %%r14\n\t"
                "popq   %%r13\n\t"
                "popq   %%r12\n\t"
                "popq   %%rbp\n\t"
                "popq   %%rax\n\t"
                "popq   %%rdx\n\t"
                "popq   %%rcx\n\t"
                "popq   %%rbx\n\t"
                "popq   %%rsi\n\t"
                "popq   %%rdi\n\t"
                "iretq"
                :
                :[rsp]"m"(new_thread->rsp)
                :"memory");
        __builtin_unreachable();
    }
    else if ( new_thread->return_type == FUNCTION )
    {
        __asm__ volatile(
                "movq   %[rsp], %%rsp\n\t"
                "sti\n\t"
                "popq   %%r15\n\t"
                "popq   %%r14\n\t"
                "popq   %%r13\n\t"
                "popq   %%r12\n\t"
                "popq   %%rbp\n\t"
                "popq   %%rbx\n\t"
                "retq"
                :
                :[rsp]"m"(new_thread->rsp)
                :"memory");
        __builtin_unreachable();
    }
    __builtin_unreachable();
}

__attribute__((noreturn))
static inline void switch_to_halt( size_t const core_id)
{
    __asm__ volatile(
            "movq   %[rsp], %%rsp\n\t"
            "sti\n"
            "1:\n\t"
            "hlt\n\t"
            "jmp    1b"
            :
            :[rsp]"r"(&halt_stacks[core_id+1])
            :"memory");
    __builtin_unreachable();
}
