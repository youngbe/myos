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
    .text
    .p2align 5
    .globl timer_isr
timer_isr:
    push_all_registers

    // 上锁
    movq    $1, %rbx
1:
    xorq    %rax, %rax
    cmpxchgq    %rbx, .Lsched_threads_mutex(%rip)
    jne     1b

    cmpq    $0, index_sched_threads(%rip)
    je     1f

    movq    %rsp, %rdi
    call    timer_isr_helper
    movq    %rax, %rsp


1:
    movq    $0, .Lsched_threads_mutex(%rip)
    xorq    %rdx, %rdx
    xorq    %rax, %rax
    movl    $0x80b, %ecx
    wrmsr
    pop_all_registers
    iretq


    .data
    .p2align 5
.Lsched_threads_mutex:
    .quad 0
