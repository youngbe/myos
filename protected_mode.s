    .code16
    .section .text
.Lstart:
    // 此Bootloader需要保证%ss %cs的值一直为0，直到进入linux内核为止
    // 初始化
    cli
    # 参考grub2源码：有一些BIOS进来后CS:IP=0x7c0:0，需要通过远跳转指令来修正
    ljmpl $0, $1f
1:
    xorl    %eax, %eax
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    movw    %ax, %ds
    movw    %ax, %es
    movl    $0x7c00, %esp
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
    cli

    lgdt    .Lgdt

    movl    %cr0, %eax
    orl     $1, %eax
    movl    %eax, %cr0
    ljmpl   $(.Lgdt1-.Lgdt0), $1f
    .code32
1:
    movl    $(.Lgdt3-.Lgdt0), %eax
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
    movl    $(.Lgdt4-.Lgdt0), %eax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    ljmp    $(.Lgdt2-.Lgdt0), $1f
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

#movl    $0xb8000+( 80 * 4+ 5 ) * 2, %eax
#movl    $'X'+(0xc00), (%eax)

    movl    $0xb800, %eax
    movw    %ax, %es
    movw    $( 80 * 6+ 5 ) * 2, %ax
    movl    $'Q'+(0xc00), %es:(%eax)
    // 程序结束
    jmp .Lerror
    jmp .



    // gdt
.Lgdt0:
    .quad 0
.Lgdt1:
    // 32位代码段
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10011011
    .byte 0b11001111
    .byte 0
.Lgdt2:
    // 16位代码段
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10011011
    .word 0
.Lgdt3:
    // 32位数据段
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10010011
    .byte 0b11001111
    .byte 0
.Lgdt4:
    // 16位数据段
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10010011
    .word 0

.Lgdt:
    .word .Lgdt-.Lgdt0-1
    .long .Lgdt0


.Lclear:
    #movw    $0x1000, %ax
    #movw    %ax, %es
    xorl    %eax, %eax
    xorl    %ebx, %ebx
    xorl    %ecx, %ecx
    xorl    %edx, %edx
    xorl    %esi, %esi
    xorl    %edi, %edi
    xorl    %ebp, %ebp
    movw    %ax, %ds
    movw    %ax, %es
    cld
    clc
    ret

1:
    .ascii "error!"
2:
.Lerror:
    call    .Lclear
    movw    %ax, %es
    movw    $1b, %bp
    movw    $(2b-1b), %cx
    movb    $0x13, %ah
    movb    $0b00001111, %bl
    int     $0x10
    jmp     .


    .fill 510- (. - .Lstart), 1, 0
    .byte   0x55
    .byte   0xaa
