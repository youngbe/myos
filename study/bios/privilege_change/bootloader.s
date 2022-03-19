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

    # 下一个要读取的逻辑扇区号，0号已经被读取到0x7c00
.Ldata_sector_num:
    .long 1


    // gdt
    # 在 NULL Descripter 上存放gdt，而不是全0
    # 根据 https://wiki.osdev.org/GDT_Tutorial#What_to_Put_In_a_GDT 在 NULL Descripter 上存放数据是合法的，Linux内核也是在 NULL Descripter 上存放gdt
.Lgdt_null:
.Lgdt_ptr:
    .word .Lgdt_end - .Lgdt_null -1
    .long .Lgdt_null
    .word 0
.Lgdt_code64:
    .long 0
    .byte 0
    .byte 0b10011010
    .byte 0b00100000
    .byte 0
.Lgdt_code64_user:
    .long 0
    .byte 0
    .byte 0b11111010
    .byte 0b00100000
    .byte 0
.Lgdt_data64_user:
    .long 0
    .byte 0
    .byte 0b11110010
    .word 0
.Lgdt_called_gate64:
    .word .Lback_to_su
    .word .Lgdt_code64-.Lgdt_null
    .byte 0
    .byte 0b11101100
    .word 0
    .quad 0
.Lgdt_tss64:
    .word 0x67
    # base 0:15
    .word .Ltss
    # base 16:23
    .byte 0
    .byte 0b10001001
    .byte 0
    # base 24:31
    .byte 0
    # base 32:63
    .long 0
    .long 0
.Lgdt_end:

.Ltss:
    .long 0
    .quad 0x50000
    .quad 0x0
    .quad 0x0
    .quad 0
    .quad 0x0
    .quad 0x0
    .quad 0x0
    .quad 0x0
    .quad 0x0
    .quad 0x0
    .quad 0x0
    .quad 0
    .long 0

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

    //检查一个扇区是不是512字节
    # qemu不支持此功能
    subw    $0x1e, %sp
    movw    %ss, %ax
    movw    %ax, %ds
    movw    %sp, %si
    movb    $0x48, %ah
    movb    $0x80, %dl
    int     $0x13
    jc      .Lerror
    testb   %ah, %ah
    jne     .Lerror
    cmpw    $0x200, %ss:24(%esp)
    jne     .Lerror
    addw    $0x1e, %sp
    call    .Lclear

    //读取Bootloader剩余部分
    # 前 65 个扇区为bootloader
    # 第 65 个扇区(从0开始数)为kernel_size
    # 第 66 个扇区开始为kernel
    movw    $65, %ax
    movl    $0x7e00000, %edx
    call    .Lread_hdd

    jmp     .Lpart2

    //参数：读取扇区数量%ax，保存位置segment:offset : %edx
.Lread_hdd:
    andl    $0xffff, %eax

    # 部分bios(比如vmware)的拓展读取磁盘服务最大一次只能读127个扇区
    # https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=42h:_Extended_Read_Sectors_From_Drive
    cmpw    $0x7f, %ax
    ja      .Lerror

    xorl    %ecx, %ecx
    movw    %cx, %ds

    subw    $0x10, %sp
    movw    $0x10, (%esp)
    movw    %ax, 2(%esp)
    movl    %edx, 4(%esp)
    movl    .Ldata_sector_num, %edx
    movl    %edx, 8(%esp)
    # 这里 %ecx 应该为0
    movl    %ecx, 12(%esp)

    addl    %eax, .Ldata_sector_num

    movw    %ss, %dx
    movw    %dx, %ds
    movl    %esp, %esi
    movb    $0x80, %dl
    movb    $0x42, %ah
    int     $0x13
    jc      .Lerror
    testb   %ah, %ah
    jne     .Lerror
    addw    $0x10, %sp
    jmp     .Lclear

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

    .fill 510-( . - _start ), 1, 0
    .byte 0x55
    .byte 0xaa

.Lpart2:

    // CPU检测
    # 是否支持 cpuid 指令
    # https://wiki.osdev.org/CPUID
    pushfl
    pushfl
    xorl    $0x00200000, (%esp)
    popfl
    pushfl
    popl    %eax
    xorl    (%esp), %eax
    popfl
    testl   $0x00200000, %eax
    jz      .Lerror

    movl    $1, %eax
    cpuid
    # 存在 msr 寄存器
    testb   $(1<<5), %dl
    jz      .Lerror
    # 存在 Local APIC
    testw   $(1<<9), %dx
    jz      .Lerror
    # 支持 x2APIC
    testl   $(1<<21), %ecx
    jz      .Lerror
    # 支持 TSC_Deadline
    testl   $(1<<24), %ecx
    jz      .Lerror
    movl    $0x1b, %ecx
    # APIC is enabled
    rdmsr
    testw   $(1<<11), %ax
    jz      .Lerror
    # If CPUID.06H:EAX.ARAT[bit 2] = 1, the processor’s APIC timer runs at a constant rate regardless of P-state transitions and it continues to run at the same rate in deep C-states.
    movl    $6, %eax
    cpuid
    testb   $(1<<2), %al
    jz      .Lerror

    call    .Lclear


    //进入保护模式的初始化
    # 打开A20以及lgdt
    movw    $0x2401, %ax
    int     $0x15
    jc      .Lerror
    testb   %ah, %ah
    jnz     .Lerror
    call    .Lclear


    // 准备64位长模式的页表
    # 初始化页表 0x20000~0x21fff 共 8kb
    # 0x20000~0x20fff PML4
    # 0x21000~0x21fff PDPT 1GB页表
    movw    $0x2000, %cx
    movw    %cx, %es
    movl    $( 0x21000 + ( (1<<0) | (1<<1) | (1<<2) ) ) , %ebx
    movl    %ebx, %es:(%di)
    addw    $4, %di
    movw    $0x3ff, %cx
    rep;    stosl

    movl    $( (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<7) ), %es:(%di)
    addw    $3, %di

    movw    $512, %cx
    xorl    %ebx, %ebx
1:
    movl    %ebx, %es:(%di)
    movl    $( ((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<7))<<8 ), %es:4(%di)
    addw    $0x40, %bx
    addw    $8, %di
    loopw   1b
    # 结束时，受污染寄存器：%si %di %es %bx



    cli

    // 进入64位长模式
    lgdtl   .Lgdt_ptr

    # 设置 %cr3
    movl    $0x20000, %eax
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

    call    .Lclear

    # 设置 %cr0 的 PE位 和 PG位
    movl    %cr0, %ecx
    orl     $( (1 << 31) | (1 << 0) ), %ecx
    movl    %ecx, %cr0

    // 进入内核
    ljmpl   $(.Lgdt_code64 - .Lgdt_null), $1f
    .code64
1:
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    movw    %ax, %ds
    movw    %ax, %es

    movw    $( .Lgdt_tss64 - .Lgdt_null ), %ax
    ltrw    %ax

    # %ss
    pushq   $( .Lgdt_data64_user - .Lgdt_null + 0b11 )
    # %rsp
    pushq   $0x60000
    # %cs
    pushq   $( .Lgdt_code64_user - .Lgdt_null + 0b11 )
    # %rip
    pushq   $.Lgo_to_user
    movb    $'k', 0xb8006
    lretq


.Lgo_to_user:
    movb    $'x', 0xb8000
    # lcallq
    rex.W lcalll *.Lback_to_su_address
    lcalll  *.Lback_to_su_address2
    movb    $'z', 0xb8004
    jmp     .

.Lback_to_su:
    movb    $'y', 0xb8002
    lretq

.Lback_to_su_address:
    .quad 0x0
    .quad .Lgdt_called_gate64 - .Lgdt_null
.Lback_to_su_address2:
    .long 0x0
    .long .Lgdt_called_gate64 - .Lgdt_null
