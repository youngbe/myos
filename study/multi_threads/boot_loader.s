# 地址 0-0x7c00
# 此段不会被加载到内存执行，只是通过标签获取地址
    .section .bss.startup
    .fill 0x7c00- ( .Lheap_end-.Lstack_end_and_heap_start ), 1, 0
.Lstack_end_and_heap_start:
#线程上下文
.Lthread_context:
    # %ss 0
    .word 0
    # %esp 2
    .long 0

# 原时间中断地址
.Loriginal_timer_int_address:
    .long 0
.Lheap_end:



    # 此处地址0x7c00，以下部分将会被加载到内存执行
    .section .text.startup
    .code16
    .globl _start
_start:
    ljmpl $0, $.Lreal_start
.Lreal_start:
    cli
    xorl    %eax, %eax
    movw    %ax, %ss
    movl    $.Lstack_end_and_heap_start, %esp
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    sti
    call    .Lclear

# 准备线程2的上下文
    movw    $0x1000, %ax
    movw    %ax, %ds
    movw    $-1, %ax
    pushfw
    movw    %ss:(%esp), %bx
    movw    %bx, -1(%eax)
    addw    $2, %sp
    movw    %cs, -3(%eax)
    movw    $.Lthread2_start, -5(%eax)
    movw    %ds, %cs:.Lthread_context
    movl    $( (-42)&0xffff ), %cs:.Lthread_context+2
# 准备时钟中断
    movl    %cs:0x8*4, %eax
    movl    %eax, %cs:.Loriginal_timer_int_address
    movl    $.Ltime_int, %cs:0x8*4

    movw    $0xb800, %ax
    movw    %ax, %ds
    xorl    %eax, %eax
1:
    movb    $'x', (%eax)
    addw    $2, %ax
    movl    $0x3ffffff, %ecx
2:
    loopl   2b
    jmp     1b

    jmp     .

# 时钟中断，切换上下文
.Ltime_int:
    pushl   %eax
    pushl   %ebx
    pushl   %ecx
    pushl   %edx
    pushl   %esi
    pushl   %edi
    pushl   %ebp
    pushw   %ds
    pushw   %es
    pushw   %fs
    pushw   %gs
    # 省略以下寄存区的保存
    # %st
    # %mm0-7
    # %xmm0-7
    # %db0-7
    # %tr6-7


    movw    %cs:.Lthread_context, %ax
    movl    %cs:.Lthread_context+2, %ebx
    movw    %ss, %cs:.Lthread_context
    movl    %esp, %cs:.Lthread_context+2
    movw    %ax, %ss
    movl    %ebx, %esp

    popw    %gs
    popw    %fs
    popw    %es
    popw    %ds
    popl    %ebp
    popl    %edi
    popl    %esi
    popl    %edx
    popl    %ecx
    popl    %ebx
    popl    %eax
    ljmp    %cs:*.Loriginal_timer_int_address

.Lclear:
    xorl    %eax, %eax
    xorl    %ebx, %ebx
    xorl    %ecx, %ecx
    xorl    %edx, %edx
    xorl    %esi, %esi
    xorl    %edi, %edi
    xorl    %ebp, %ebp
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    cld
    clc
    ret

.Lthread2_start:
    movw    $0xb000, %ax
    movw    %ax, %ds
    movl    $0x8f9e, %eax
1:
    movb    $'y', (%eax)
    subw    $2, %ax
    movl    $0x3ffffff, %ecx
2:
    loopl   2b
    jmp     1b

    .fill 510-( . - _start ), 1, 0
    .byte 0x55
    .byte 0xaa
