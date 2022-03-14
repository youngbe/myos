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


    //读取Bootloader剩余部分
    # 前 66 个扇区为bootloader
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


.Ltimer_interrupt:
    incb    %bl
    movb    %bl, 0x0
    wrmsr
.Lspurious_interrupt:
    iret



.Lpart2:
    // 准备时钟中断 int $0x30
    movl    $.Ltimer_interrupt, 0x30*4
    // 虚假中断 int $0x32
    movl    $.Lspurious_interrupt, 0x32*4


    cli
    // 禁用 PIC 8259 中断
    movb    $0x11, %al
    outb    %al, $0x20
    # io_wait
    xorb    %al, %al
    outb    %al, $0x80
    movb    $0x11, %al
    outb    %al, $0xa0
    xorb    %al, %al
    outb    %al, $0x80
    movb    $0x20, %al
    outb    %al, $0x21
    xorb    %al, %al
    outb    %al, $0x80
    movb    $0x28, %al
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
    movl    $0x80F, %ecx
    rdmsr
    andl    $0xFFFFEC00, %eax
    orl     $( (1<<8) + 0x32), %eax
    wrmsr

#MFENCE;LFENCE
    
    movl    $0x83e, %ecx
    rdmsr
    andl    $0xFFFFFFF0, %eax
    orl     $( 0b1010 ), %eax
    wrmsr

#MFENCE;LFENCE

    # timer
    movl    $0x832, %ecx
    rdmsr
    andl    $0xFFF8FF00, %eax
    orl     $( (0b01<<17) + 0x30 ), %eax
    wrmsr

#MFENCE;LFENCE

    # count
    movl    $491520, %eax
    movl    $0x838, %ecx
    wrmsr

    movw    $0xb800, %ax
    movw    %ax, %ds
    xorl    %edx, %edx
    xorl    %eax, %eax
    movl    $0x80b, %ecx
    movb    $'!', %bl
    sti
1:
    hlt
    jmp     1b
