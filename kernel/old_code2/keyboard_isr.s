    .code64
    .section .text
    .globl keyboard_isr
1:
    .byte '!'
keyboard_isr:
    pushq   %rax
    pushq   %rcx
    pushq   %rdx
    inb     $0x60, %al
    movb    %al, 0xb800c
    movb    1b(%rip), %al
    movb    %al, 0xb8010
    incb    1b(%rip)
    xorq    %rdx, %rdx
    xorq    %rax, %rax
    movl    $0x80b, %ecx
    wrmsr
    popq    %rdx
    popq    %rcx
    popq    %rax
    iretq
