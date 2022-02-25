# 地址 0-0x7c00
# 此段不会被加载到内存执行，只是通过标签获取地址
    .section .bss.startup
    .fill 0x7c00- ( .Lheap_end-.Lstack_end_and_heap_start ), 1, 0
.Lstack_end_and_heap_start:
.Lheap_end:

    # 此处地址0x7c00，以下部分将会被加载到内存执行
    .section .text.startup
    .code16
    .globl _start
_start:
    # 重置 %cs 和 %eip
    # 抄自grub：https://git.savannah.gnu.org/gitweb/?p=grub.git;a=blob;f=grub-core/boot/i386/pc/boot.S#l227
    ljmpl $0, $.Lreal_start

    // gdt
.Lgdt_null:
    .quad 0
.Lgdt_code64:
    // 64位代码段
    .long 0
    .byte 0
    .byte 0b10011010
    .byte 0b00100000
    .byte 0
.Lgdt_data64:
    // 64位数据段
    .long 0
    .byte 0
    .byte 0b10010010
    .word 0

    # 抄的，但并不知道为什么需要对齐
    # https://wiki.osdev.org/Entering_Long_Mode_Directly#Switching_to_Long_Mode
    .p2align 2
    .word 0

.Lgdt_ptr:
    .word .Lgdt_ptr-.Lgdt_null-1
    .long .Lgdt_null

.Lreal_start:
    xorl    %eax, %eax
    cli
    movw    %ax, %ss
    movl    $.Lstack_end_and_heap_start, %esp
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    sti
    call    .Lclear

    # 将 0x10000~0x12fff 12kb 内存 清0
    movw    $0x1000, %cx
    movw    %cx, %es
    movw    $0xc00, %cx
    rep     stosl


    movl    $0x11003 , %eax
    movl    %eax, %es:0x0

    movl    $0x12003 , %eax
    movl    %eax, %es:0x1000

    movl    $0x13003 , %eax
    movl    %eax, %es:0x2000

    movw    $0x200, %cx
    movl    $3, %eax
1:
    movl    %eax, %es:(%edi)
    addw    $8, %di
    addl    $0x1000, %eax
    loopl   1b

    # 打开A20
    movw    $0x2401, %ax
    int     $0x15
    jc      .Lerror
    testb   %ah, %ah
    jnz     .Lerror
    call    .Lclear

    cli

    lgdt    .Lgdt_ptr

    # 设置 %cr3
    movl    $0x10000, %eax
    movl    %eax, %cr3

    # 设置 %cr4 的PAE位
    movl    %cr4, %eax
    orl     $( 1<<5 ), %eax
    movl    %eax, %cr4

    # 设置 EFER 寄存器的LME位
    movl    $0xC0000080, %ecx
    rdmsr
    orl     $( 1 << 8 ), %eax
    wrmsr

    # 设置 %cr0 的 PE位 和 PG位
    movl    %cr0, %eax
    orl     $( (1 << 31) | (1 << 0) ), %eax
    movl    %eax, %cr0

    ljmpl   $(.Lgdt_code64 - .Lgdt_null), $1f
    .code64
1:
    movw    $(.Lgdt_data64 - .Lgdt_null ), %ax
    movw    %ax, %ss
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs

    movq    $0xb8000, %r12
    movb    $'x', (%r12)
    jmp     .

    .code16
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

.Lerror:
    movw    $0xb800, %ax
    movw    %ax, %ds
    movb    $'e', 0x0
    jmp     .


    .fill 510-( . - _start ), 1, 0
    .byte   0x55
    .byte   0xaa
