    .code64
    .section .text
    .globl timmer_isr
timmer_isr:
    pushaq

    // 给 schedulable_thread_list 上锁
    movq    $1, %rbx
1:
    xorq    %rax, %rax
    cmpxchgq    %rbx, schedulable_thread_list_mutex
    jne     1b

    cmpq    $0, schedulable_thread_list_index
    jne     1f
    movq    $0, schedulable_thread_list_mutex
    popaq
    iretq
1:

    call    switch_helper
    movq    $0, schedulable_thread_list_mutex
    ltrw    %ax

    movq    %rax, %rsp
    popaq
    iretq
