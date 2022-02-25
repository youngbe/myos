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
.Lgdt_code32:
    // 32位代码段
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10011011
    .byte 0b11001111
    .byte 0
.Lgdt_code16:
    // 16位代码段
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10011011
    .word 0
.Lgdt_data32:
    // 32位数据段
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10010011
    .byte 0b11001111
    .byte 0
.Lgdt_data16:
    // 16位数据段
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10010011
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

    //打开 A20
    movw    $0x2401, %ax
    int     $0x15
    jc      .Lerror
    testb   %ah, %ah
    jnz     .Lerror
    call    .Lclear

    //进入保护模式
    # 进入保护模式后没有初始化中断向量表，因此要全程关中断
    cli

    lgdt    .Lgdt_ptr

    movl    %cr0, %eax
    orl     $1, %eax
    movl    %eax, %cr0
    ljmpl   $(.Lgdt_code32-.Lgdt_null), $1f
    .code32
1:
    movl    $(.Lgdt_data32-.Lgdt_null), %eax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    xorl    %eax, %eax
    xorl    %ebx, %ebx
    xorl    %ecx, %ecx
    xorl    %edx, %edx
    xorl    %esi, %esi
    xorl    %edi, %edi
    xorl    %ebp, %ebp
    #sti
    
    movl    $0xb8000+ ( 80 * 2+ 0 ) * 2, %eax
    movl    $'P'+(0xc00),(%eax)

    // 返回实模式
    movl    $(.Lgdt_data16-.Lgdt_null), %eax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    ljmpl    $(.Lgdt_code16-.Lgdt_null), $1f
    .code16
1:
    movl    %cr0, %eax
    andl    $0xfffffffe, %eax
    movl    %eax, %cr0
    ljmpl   $0, $1f
1:

    xorl    %eax, %eax
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    movw    %ax, %ds
    movw    %ax, %es
    sti
    call    .Lclear

    movl    $0xb800, %eax
    movw    %ax, %es
    movw    $( 80 * 6+ 5 ) * 2, %ax
    movl    $'Q'+(0xc00), %es:(%eax)
    // 程序结束
    jmp .


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

1:
    .ascii "error!"
2:
.Lerror:
    movl    $0x1300, %eax
    movl    $0b00001111, %ebx
    movl    $(2b-1b), %ecx
    xorl    %edx, %edx
    movw    %dx, %es
    movl    $1b, %ebp
    int     $0x10
    jmp     .


    .fill 510- (. - _start), 1, 0
    .byte   0x55
    .byte   0xaa
