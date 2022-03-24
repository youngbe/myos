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
    ljmpl $0, $1f
1:
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

    // 打开A20
    movw    $0x2401, %ax
    int     $0x15
    jc      .Lerror
    testb   %ah, %ah
    jnz     .Lerror
    call    .Lclear

    //检查一个扇区是不是512字节
    # qemu不支持此BIOS扩展
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

    // 硬件兼容性检测及设置

    # 1. 检测是否支持 cpuid 指令
    # https://wiki.osdev.org/CPUID
    pushfl
    pushfl
    xorl    $0x00200000, %ss:(%esp)
    popfl
    pushfl
    popl    %eax
    xorl    %ss:(%esp), %eax
    popfl
    testl   $0x00200000, %eax
    jz      .Lerror

    movl    $1, %eax
    cpuid
    # 2. 检测是否存在 msr 寄存器
    testb   $(1<<5), %dl
    jz      .Lerror
    # 3. 存在 Local APIC
    testw   $(1<<9), %dx
    jz      .Lerror
    # 4. 支持 x2APIC
    testl   $(1<<21), %ecx
    jz      .Lerror
    # 5. 支持 TSC_Deadline
    testl   $(1<<24), %ecx
    jz      .Lerror
    movl    $0x1b, %ecx
    # 6. APIC is enabled
    rdmsr
    testw   $(1<<11), %ax
    jz      .Lerror
    # 7. 选自英特尔3a卷
    # If CPUID.06H:EAX.ARAT[bit 2] = 1, the processor’s APIC timer runs at a constant rate regardless of P-state transitions and it continues to run at the same rate in deep C-states.
    movl    $6, %eax
    cpuid
    testb   $(1<<2), %al
    jz      .Lerror

    # 8. clear CR4.CET CR4.PKS CR4.PKE
    movl    %cr4, %eax
    testl   $( (1<<23) | (1<<22) | (1<<24) ), %eax
    jz      1f
    andl    $( ~( (1<<23) | (1<<22) | (1<<24) ) ), %eax
    movl    %eax, %cr4
1:

    # clear CR0.WP
    movl    %cr0, %eax
    testl   $(1<<16), %eax
    jz      1f
    andl    $( ~(1<<16) ), %eax
    movl    %eax, %cr0
1:

    call    .Lclear


    //读取Bootloader剩余部分
    # 前 66 个扇区为bootloader
    movw    $65, %ax
    movl    $0x7e00000, %edx
    call    .Lread_hdd

    jmp     .Lpart2





    // read_hdd：读取磁盘
    // 参数：读取扇区数量%ax，保存位置segment:offset : %edx

    # 下一个要读取的逻辑扇区号，0号已经被读取到0x7c00
1:
    .long 1
.Lread_hdd:
    andl    $0xffff, %eax

    # 部分bios(比如vmware)的拓展读取磁盘服务最大一次只能读127个扇区
    # https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=42h:_Extended_Read_Sectors_From_Drive
    cmpw    $0x7f, %ax
    ja      .Lerror

    xorl    %ecx, %ecx
    movw    %cx, %ds

    subw    $0x10, %sp
    movw    $0x10, %ss:(%esp)
    movw    %ax, %ss:2(%esp)
    movl    %edx, %ss:4(%esp)
    movl    1b, %edx
    movl    %edx, %ss:8(%esp)
    # 这里 %ecx 应该为0
    movl    %ecx, %ss:12(%esp)

    addl    %eax, 1b

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
    # clear screen
    call    .Lclear
    movb    $0x07, %ah
    movb    $0b00001111, %bh
    movw    $0x184f, %dx
    int     $0x10
    call    .Lclear
    movb    $0x13, %ah
    movb    $0b00001111, %bl
    movw    $(2b-1b), %cx
    movw    $1b, %bp
    int     $0x10
    jmp     .

    .fill 510-( . - _start ), 1, 0
    .byte 0x55
    .byte 0xaa


    // gdt
    # 在 NULL Descripter 上存放gdt，而不是全0
    # 根据 https://wiki.osdev.org/GDT_Tutorial#What_to_Put_In_a_GDT 在 NULL Descripter 上存放数据是合法的，Linux内核也是在 NULL Descripter 上存放gdt
    # 根据英特尔白皮书3a卷 3.5.1 ，应该对齐8字节以获得更好的性能
    .balign 8
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
    .word .Lsupervisor_service
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
    # 指定切换到低特权级时的栈(%rsp)
    .quad 0x50000
    .quad 0x0
    .quad 0x0
    .quad 0
    # IST 字段，目前没有用到
    .quad 0x0
    .quad 0x0
    .quad 0x0
    .quad 0x0
    .quad 0x0
    .quad 0x0
    .quad 0x0
    .quad 0
    # I/O Map base address，目前不知道干什么用的
    .long 0

.Lpart2:



    // 准备64位长模式的页表
    # 初始化页表 0x20000~0x21fff 共 8kb
    # 0x20000~0x20fff PML4
    # 0x21000~0x21fff PDPT 1GB页表
    movw    $0x2000, %cx
    movw    %cx, %es
    # 从%cr3到找到物理页的过程中，必须所有U/S位都是1，它才是用户页，否则是系统页
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

    movb    $'s', 0xb8008

    # 加载TSS，务必在进入64位后再加载64位TSS
    movw    $( .Lgdt_tss64 - .Lgdt_null ), %ax
    ltrw    %ax

    # 指定切换到用户态时的%ss
    # 0b11指定特权级为用户态，这个是必须的，且必须使用用户态数据段
    pushq   $( .Lgdt_data64_user - .Lgdt_null + 0b11 )
    # 指定切换到用户态时的%rsp
    pushq   $0x60000
    # 指定切换到用户态时的%cs
    # 0b11指定特权级为用户态
    pushq   $( .Lgdt_code64_user - .Lgdt_null + 0b11 )
    # 指定切换到用户态时的%rip
    pushq   $.Luser_start
    # 切换到用户态（从.Luser_start开始执行）
    lretq


.Luser_start:
    movb    $'x', 0xb8000

    movb    $'y', %al
    movq    $0xb8002, %rdi
    pushq   $(.Lgdt_called_gate64 - .Lgdt_null)
    pushq   $0xfffffff
    # (%rsp) == lcallq的目标%rip
    # 8(%rsp) == lcallq的目标%cs
    # rex.W lcalll == lcallq (gnu as 的缺陷，clang 就可以编译lcallq，两者的二进制代码相同)
    rex.W lcalll *(%rsp)
    # 因为lcall的是一个门，所以目标%rip将被忽略，填什么都行
    addq    $16, %rsp

    movb    $'z', %al
    movq    $0xb8004, %rdi
    subq    $8, %rsp
    movl    $0xffffff, (%rsp)
    movl    $(.Lgdt_called_gate64 - .Lgdt_null), 4(%rsp)
    # (%rsp) == lcalll的目标%rip
    # 4(%rsp) == lcalll的目标%cs
    lcalll  *(%rsp)
    # 因为lcall的是一个门，所以目标%rip将被忽略，填什么都行
    addq    $8, %rsp

    movb    $'u', 0xb8006
    jmp     .




# 高特权级服务
.Lsupervisor_service:
    # 当从低特权级(特权级数字较大)lcall进来时，CPU将做以下的事：
    # 1. 从TSS中选取新的栈(%rsp)，修改%rsp的值
    # 2. %ss 变为 0
    # 3. 将lcall时的 %ss 入栈
    # 4. 将lcall时的 %rsp 入栈
    # 5. 将lcall时的 %cs 入栈
    # 6. 将lcall时的 %rip 入栈
    movb    %al, (%rdi)
    lretq
