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

    # 内核代码接下来要加载位置
.Ldata_protected_mode_code:
    .long 0x100000

.Ldata_memory_map_size:
    .long 0

    /*
    // gdt
    # 八字节对齐也是抄自Linux源代码
    .balign	8
    # 抄自Linux源代码，使第一个段描述符(原本应该使八字节全0)，存放gdt
    # 从而节省空间
    # https://elixir.bootlin.com/linux/v5.16.11/source/arch/x86/boot/compressed/head_64.S#L701
    */

    // 上面一段注释还没搞明白，暂时弃用
    // gdt
    .balign 8
.Lgdt_null:
    .quad 0
.Lgdt_code64:
    .long 0
    .byte 0
    .byte 0b10011010
    .byte 0b00100000
    .byte 0
.Lgdt_data64:
    .long 0
    .byte 0
    .byte 0b10010010
    .word 0
.Lgdt64_end:
.Lgdt_code32:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10011011
    .byte 0b11001111
    .byte 0
.Lgdt_code16:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10011011
    .word 0
.Lgdt_data32:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10010011
    .byte 0b11001111
    .byte 0
.Lgdt_data16:
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10010011
    .word 0

.Lgdt_ptr:
    .word .Lgdt_ptr - .Lgdt_null -1
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

    //检查一个扇区是不是512字节
    # qemu不支持int $0x13, %ah=$0x48
    #subw    $0x1e, %sp
    ##movw    %ss, %ax
    ##movw    %ax, %ds
    #movw    %sp, %si
    #movb    $0x48, %ah
    #movb    $0x80, %dl
    #int     $0x13
    #jc      .Lerror
    #testb   %ah, %ah
    #jne     .Lerror
    #cmpw    $0x200, 24(%esp)
    #jne     .Lerror
    #addw    $0x1e, %sp
    #call    .Lclear

    //读取Bootloader剩余部分
    # 因为我的Bootloader有点大，512字节装不下，我写了两个扇区
    // 同时读取内核头扇区
    movw    $2, %ax
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


# 从 0x30000拷贝 %eax 个字节的数据到 *.Ldata_protected_mode_code
.Lcopy_to_high:
    pushl   %eax
    call    .Lclear
    cli
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

    popl    %ecx
    movl    .Ldata_protected_mode_code, %edi
    addl    %ecx, .Ldata_protected_mode_code
    movb    %cl, %dl
    addb    $0b11, %dl
    shrl    $2, %ecx
    movl    $0x30000, %esi
    rep;    movsl
    movb    %dl, %cl
    rep;    movsb

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
    jmp     .Lclear


    .fill 510-( . - _start ), 1, 0
    .byte 0x55
    .byte 0xaa

.Lpart2:

    // 侦测内存分布
    # https://wiki.osdev.org/Detecting_Memory_(x86)
    // 第一次调用：%eax=0xe820, %ebx=0, %ecx=24, %edx=smap, %es:%edi = 地址
    // 返回 cflag位，%eax=smap, %ebx保留，%cl=位数
    // 错误判断： cflag位，%eax=smap, %ebx!=0
    // 之后的调用：%eax=0xe820，%ebx保留，%ecx=24，%edx=smap，%es:%edi = 地址
    // 返回：cflag位，%ebx保留，%cl=位数
    // 结束判断：cflag位，%ebx
    movw    $0x2000, %ax
    movw    %ax, %es
    pushw   %es
    pushl   %edi
    movw    $0xe820, %ax
    movb    $24, %cl
    movl    $0x534d4150, %edx
    int     $0x15
    jc      .Lerror
    cmpl    $0x534d4150, %eax
    jne     .Lerror
    testl   %ebx, %ebx
    jz      .Lerror





    // 每次获取完一个条目后的处理
5:
    pushl   %ebx
    pushw   %cx
    call    .Lclear
    popw    %cx
    popl    %ebx
    popl    %edi
    popw    %es
    // 判断是不是有效条目
    # if %cl != 24
    cmpb    $24, %cl
    je      1f
        # if %cl != 20
        # goto error
        cmpb    $20, %cl
        jne     .Lerror
    jmp     2f
    # else
1:
        # if clear位 为 0
        # 无效条目
        testb   $1, %es:20(%edi)
        jz      3f
2:
    cmpl    $0, %es:8(%edi)
    jne     4f
    cmpl    $0, %es:12(%edi)
    je      3f
    # 有效条目
4:
    movb    %cl, %es:24(%edi)
    addw    $25, %di
    incl    .Ldata_memory_map_size
    # 无效条目
3:



    // 读取下一个条目
    # 没有下一个条目了
    testl   %ebx, %ebx
    jz      6f
    # 存放条目空间不足
    cmpw    $(25*2621), %di
    je      .Lerror
    pushw   %es
    pushl   %edi
    movl    $0xe820, %eax
    movl    $24, %ecx
    movl    $0x534d4150, %edx
    int     $0x15
    jnc     5b
    addw    $6, %sp
6:
    call    .Lclear

    //打开 A20
    movw    $0x2401, %ax
    int     $0x15
    jc      .Lerror
    testb   %ah, %ah
    jnz     .Lerror
    call    .Lclear

    cli
    lgdtl   .Lgdt_ptr
    sti

    movl    0x8000, %eax
    # 现在%eax存放了需要读取的扇区数

    # while %eax > 0x7f
2:
    cmpl    $0x7f, %eax
    jbe     1f
    pushl   %eax
    movw    $0x7f, %ax
    movl    $0x30000000, %edx
    call    .Lread_hdd
    movl    $0xfe00, %eax
    call    .Lcopy_to_high
    popl    %eax
    subl    $0x7f, %eax
    jmp     2b

1:
    pushw   %ax
    movl    $0x30000000, %edx
    call    .Lread_hdd
    popw    %ax
    shll    $9, %eax
    call    .Lcopy_to_high

    # 初始化页表 0x10000~0x11fff 共 8kb
    # 0x10000~0x10fff PML4
    # 0x11000~0x11fff PDPT 1GB页表
    movw    $0x1000, %cx
    movw    %cx, %es
    movl    $( 0x11000 + ( (1<<0) | (1<<1) ) ) , %ebx
    movl    %ebx, %es:(%edi)
    addw    $4, %di
    movw    $0x3ff, %cx
    rep     stosl

    movl    $( (1<<0)|(1<<1)|(1<<7) ), %es:(%edi)
    addw    $3, %di

    movw    $512, %cx
    xorl    %ebx, %ebx
1:
    movl    %ebx, %es:(%edi)
    movl    $( ((1<<0)|(1<<1)|(1<<7))<<8 ), %es:4(%edi)
    addw    $0x40, %bx
    addw    $8, %di
    loopw   1b

    # 重新加载gdt，删去不必要的段(16位和32位)
    movw    $(.Lgdt64_end-.Lgdt_null-1), .Lgdt_ptr
    cli
    lgdtl   .Lgdt_ptr

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

    call    .Lclear

    # 设置 %cr0 的 PE位 和 PG位
    movl    %cr0, %eax
    orl     $( (1 << 31) | (1 << 0) ), %eax
    movl    %eax, %cr0

    // 进入内核
    ljmpl   $(.Lgdt_code64 - .Lgdt_null), $1f
    .code64
1:
    movw    $(.Lgdt_data64 - .Lgdt_null), %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    xorq    %rax, %rax
    jmp     0x100000
