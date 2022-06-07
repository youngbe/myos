    .text
    .code64
    .globl  return_handler_timer_int
return_handler_timer_int:
    popq   %r11
    popq   %r10
    popq   %r9
    popq   %r8
    popq   %r15
    popq   %r14
    popq   %r13
    popq   %r12
    popq   %rax
    popq   %rdx
    popq   %rcx
    popq   %rbx
    popq   %rsi
    popq   %rdi
    popq   %rbp
    iretq
    // FUNCTION
    .globl return_handler_function
return_handler_function:
    sti
    popq   %r15
    popq   %r14
    popq   %r13
    popq   %r12
    popq   %rbp
    popq   %rbx
    retq
    .globl return_handler_thread_start
return_handler_thread_start:
    popq    %rsi
    popq    %rdi
    popq    %rcx
    popq    %rsp
    movq    $( (1<<9) | (1<<1) ), %r11
    sysretq
