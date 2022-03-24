    .macro enter_protected_mode
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
    .endm
    .macro exit_protected_mode
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
    .endm


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

    /*
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
    */


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


    # 内核32位代码接下来要加载位置
.Lkernel_load_next_address:
    .long 0x100000

    // 写入内核头的 cmd line
.Ldata_cmd_line:
    .ascii "root=/dev/sdb2\0"
.Ldata_cmd_line_end:

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
.Lgdt_end:

# 从 0x30000拷贝 %eax 个字节的数据到 *.Lkernel_load_next_address
.Lcopy_to_high:
    pushl   %eax
    call    .Lclear
    cli
    enter_protected_mode

    popl    %ecx
    movl    .Lkernel_load_next_address, %edi
    addl    %ecx, .Lkernel_load_next_address
    movb    %cl, %dl
    addb    $0b11, %dl
    shrl    $2, %ecx
    movl    $0x30000, %esi
    rep;    movsl
    movb    %dl, %cl
    rep;    movsb

    // 返回实模式
    exit_protected_mode
    sti
    jmp     .Lclear








.Lpart2:

    lgdtl   .Lgdt_ptr

    //读内核第一个扇区，放到0x10000
    movw    $1, %ax
    movl    $0x10000000, %edx
    call    .Lread_hdd

    // 读取kernel setup sector
    movw    $0x1000, %bx
    movw    %bx, %ds
    movb    0x1f1, %al

    # 检查是否太多扇区，不够空间存放
    # 根据内核文档，Kernel boot sector + Kernel setup 的大小不应该超过0x8000
    # 因此setup_sector的大小不应该超过0x3f
    cmpb    $0x3f, %al
    ja      .Lerror

    # 根据内核文档，%al==0则%al==4
    testb   %al, %al
    jne     1f
    movb    $4, %al
1:
    # 将内核16进制代码拷贝到0x10200
    movl    $0x10000200, %edx
    call    .Lread_hdd

    # 设置command line
    # 把cmd_line拷贝到0x20000
    movw    $0x2000, %cx
    movw    %cx, %es
    movw    $.Ldata_cmd_line, %si
    movw    $(.Ldata_cmd_line_end-.Ldata_cmd_line),%cx
    rep;    movsb

    //set_header_fields:
    movw    $0x1000, %ax
    movw    %ax, %ds
    movb    $0xff, 0x210
    orb     $0x80, 0x211
    andb    $0xdf, 0x211
    movw    $0xfdff, 0x224
    #movb   $0x00, 0x226
    movb    $0x01, 0x227
    movl    $0x20000, 0x228


    //加载内核32位代码
    movl    0x1f4, %eax
    testl   $0x1f, %eax
    jz      1f
    addl    $0x20, %eax
1:
    shr     $5, %eax
    # 现在 %eax 是要读取的扇区数

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

    //进入内核
    call    .Lclear
    movw    $0x1000, %ax
    cli
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    movl    $0xffff, %esp
    xorl    %eax, %eax
    ljmpl   $0x1020, $0x0
