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
    call    .Lclear
    movw    %ss, %dx
    movw    %dx, %ds
    movw    %sp, %si
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
.Lgdt_end:

    # 根据英特尔白皮书3a卷 ，应该对齐8字节以获得更好的性能
    .balign 8
.Lidt_start:
    .rept 32
    .word .Lisr_empty
    .word .Lgdt_code64-.Lgdt_null
    .byte 0
    .byte 0b11101110
    .word 0
    .quad 0
    .endr

    .word .Lisr32
    .word .Lgdt_code64-.Lgdt_null
    .byte 0
    .byte 0b11101110
    .word 0
    .quad 0

    .rept 223
    .word .Lisr_empty
    .word .Lgdt_code64-.Lgdt_null
    .byte 0
    .byte 0b11101110
    .word 0
    .quad 0
    .endr
.Lidt_end:

.Lidt_ptr:
    .word .Lidt_end - .Lidt_start -1
    .long .Lidt_start


    .code64
1:
    .byte '!'
.Lisr32:
    pushq   %rax
    pushq   %rcx
    pushq   %rdx
    movb    1b, %al
    movb    %al, 0xb8004
    incb    1b
    xorq    %rdx, %rdx
    xorq    %rax, %rax
    movl    $0x80b, %ecx
    wrmsr
    popq    %rdx
    popq    %rcx
    popq    %rax
.Lisr_empty:
    iretq
    .code16


.Lpart2:



    // 准备64位长模式的页表
    # 初始化页表 0x20000~0x21fff 共 8kb
    # 0x20000~0x20fff PML4
    # 0x21000~0x21fff PDPT 1GB页表
    movw    $0x2000, %cx
    movw    %cx, %es
    movl    $( 0x21000 + ( (1<<0) | (1<<1) ) ) , %ebx
    movl    %ebx, %es:(%di)
    addw    $4, %di
    movw    $0x3ff, %cx
    rep;    stosl

    movl    $( (1<<0)|(1<<1)|(1<<3)|(1<<4)|(1<<7) ), %es:(%di)
    addw    $3, %di

    movw    $512, %cx
    xorl    %ebx, %ebx
1:
    movl    %ebx, %es:(%di)
    movl    $( ((1<<0)|(1<<1)|(1<<3)|(1<<4)|(1<<7))<<8 ), %es:4(%di)
    addw    $0x40, %bx
    addw    $8, %di
    loopw   1b
    # 结束时，受污染寄存器：%si %di %es %bx



    // 禁用 PIC 8259 中断
    // 使用 x2APIC
    cli

    movb    $0x11, %al
    outb    %al, $0x20
    # io_wait
    xorb    %al, %al
    outb    %al, $0x80

    movb    $0x11, %al
    outb    %al, $0xa0
    xorb    %al, %al
    outb    %al, $0x80


    movb    $0xf8, %al
    outb    %al, $0x21
    xorb    %al, %al
    outb    %al, $0x80

    movb    $0xf8, %al
    outb    %al, $0xa1
    xorb    %al, %al
    outb    %al, $0x80


    movb    $0x4, %al
    outb    %al, $0x21
    xorb    %al, %al
    outb    %al, $0x80

    movb    $0x2, %al
    outb    %al, $0xa1
    xorb    %al, %al
    outb    %al, $0x80


    movb    $0x1, %al
    outb    %al, $0x21
    xorb    %al, %al
    outb    %al, $0x80

    movb    $0x1, %al
    outb    %al, $0xa1
    xorb    %al, %al
    outb    %al, $0x80


    movb    $0xff, %al
    outb    %al, $0xa1
    outb    %al, $0x21

    // enable x2APIC
    movl    $0x1b, %ecx
    rdmsr
    testw   $(1<<10), %ax
    jnz     1f
    orw     $(1<<10), %ax
    wrmsr
1:
    
    xorl    %edx, %edx
   /* 
    // 软件禁用x2APIC
    movl    $0x80F, %ecx
    movl    $0x32, %eax
    wrmsr

    MFENCE;LFENCE

    # count
    movl    $0x838, %ecx
    movl    $0xffffffff, %eax
    wrmsr
    
    # divide
    movl    $0x83e, %ecx
    movl    $128, %eax
    wrmsr

    # timer
    movl    $0x832, %ecx
    rdmsr
    andl    $0xFFF8FF00, %eax
    orl     $( (0b01<<17)  + (1<<16) + 0x30 ), %eax
    wrmsr

    MFENCE;LFENCE
    */
    # Spurious Interrupt Vector Register
    movl    $0x80F, %ecx
    rdmsr
    andl    $( ~( 0x3ff+(1<<12) ) ), %eax
    orl     $( (1<<8) + 0xff), %eax
    wrmsr

    # LVT LINIT0 Register
    movl    $0x835, %ecx
    rdmsr

    # LVT LINIT1 Register
    movl    $0x836, %ecx
    rdmsr
    
    # Divide Configuration Register
    movl    $0x83e, %ecx
    rdmsr
    andl    $(~0xf), %eax
    orl     $( 0b1010 ), %eax
    wrmsr

    # LVT Timer register
    movl    $0x832, %ecx
    rdmsr
    andl    $( ~( 0xff+(7<<16) ) ), %eax
    orl     $( (0b01<<17) + 0x20 ), %eax
    wrmsr

    # count
    movl    $491520, %eax
    movl    $0x838, %ecx
    wrmsr

    // 进入64位长模式
    lgdtl   .Lgdt_ptr
    lidtl   .Lidt_ptr

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

    sti
1:
    hlt
    jmp     1b
