    .macro push_all_registers
    pushq   %rax
    pushq   %rbx
    pushq   %rcx
    pushq   %rdx
    pushq   %rsi
    pushq   %rdi
    pushq   %rbp
    pushq   %r8
    pushq   %r9
    pushq   %r10
    pushq   %r11
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15
    .endm
    .macro  pop_all_registers
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %r11
    popq    %r10
    popq    %r9
    popq    %r8
    popq    %rbp
    popq    %rdi
    popq    %rsi
    popq    %rdx
    popq    %rcx
    popq    %rbx
    popq    %rax
    .endm


    .code64
    .section .text
    .globl timer_isr
timer_isr:
    push_all_registers

    // 给 schedulable_thread_list 上锁
    movq    $1, %rbx
1:
    xorq    %rax, %rax
    cmpxchgq    %rbx, schedulable_thread_list_mutex(%rip)
    jne     1b

    cmpq    $0, schedulable_thread_list_index(%rip)
    jne     1f
    movq    $0, schedulable_thread_list_mutex(%rip)
    pop_all_registers
    iretq
1:
    call    timer_isr_helper
    movq    $0, schedulable_thread_list_mutex(%rip)
    movq    %rax, %rsp
    pop_all_registers
    iretq
