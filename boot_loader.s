    .code16
    .section .text
.Lstart:
    // 此Bootloader需要保证%ss %cs的值一直为0，直到进入linux内核为止
    // 初始化
    cli
    ljmpl $0, $.Lreal_start
.Lreal_start:
    xorl    %eax, %eax
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    movw    %ax, %ds
    movw    %ax, %es
    movl    $0x7c00, %esp
    sti
    call    .Lclear

    //检查一个扇区是不是512字节
    subw    $0x1e, %sp
    #movw    %ss, %ax
    #movw    %ax, %ds
    movw    %sp, %si
    movb    $0x48, %ah
    movb    $0x80, %dl
    int     $0x13
    jc      .Lerror
    testb   %ah, %ah
    jne     .Lerror
    cmpw    $0x200, 24(%esp)
    jne     .Lerror
    addw    $0x1e, %sp
    call    .Lclear

    //读取Bootloader剩余部分
    movw    $1, %ax
    movl    $0x7e00000, %edx
    call    .Lread_hdd

    //读内核第一个扇区
    movw    $1, %ax
    movl    $0x10000000, %edx
    call    .Lread_hdd

    // 读取kernel setup sector
    movb    %es:0x1f1, %al

    # 检查是否太多扇区，不够空间存放
    # 根据内核文档，Kernel boot sector + Kernel setup 的大小不应该超过0x8000
    # 因此setup_sector的大小不应该超过0x3f
    cmpb    $0x3f, %al
    ja      .Lerror

    testb   %al, %al
    jne     .Lread_kernel_setup_next
    movb    $4, %al
.Lread_kernel_setup_next:
    movl    $0x10000200, %edx
    call    .Lread_hdd

    jmp     .Lpart2



    //参数：读取扇区数量%ax，保存位置segment:offset : %edx
.Lread_hdd:
    andl    $0xffff, %eax
    subw    $0x10, %sp

    movw    $0x10, (%esp)
    movw    %ax, 2(%esp)
    movl    %edx, 4(%esp)
    movl    %cs:.Ldata1, %edx
    movl    %edx, 8(%esp)
    movl    $0, 12(%esp)

    addl    %eax, %cs:.Ldata1
    movw    %ss, %dx
    movw    %dx, %ds
    movl    %esp, %esi
    movl    $0x80, %edx
    movb    $0x42, %ah
    int     $0x13
    jc      .Lerror
    testb   %ah, %ah
    jne     .Lerror
    addw    $0x10, %sp
    call    .Lclear
    ret

.Lclear:
    movw    $0x1000, %ax
    movw    %ax, %es
    xorl    %eax, %eax
    xorl    %ebx, %ebx
    xorl    %ecx, %ecx
    xorl    %edx, %edx
    xorl    %esi, %esi
    xorl    %edi, %edi
    xorl    %ebp, %ebp
    movw    %ax, %ds
    cld
    clc
    ret

.Lerror:
    call    .Lclear
    movw    %ax, %es
    movw    $.Ldata2, %bp
    movw    $(.Ldata3-.Ldata2), %cx
    movb    $0x13, %ah
    movb    $0b00001111, %bl
    int     $0x10
    jmp     .




    // 下一个要读取的逻辑扇区号
    // 0, 1号扇区是boot_loader，2号扇区开始是内核
.Ldata1:
    .long 1


.Ldata2:
    .ascii "error!"


    // cmd line
.Ldata3:
    .ascii "root=/dev/sdb2\0"


    // gdt
.Ldata4:
    .quad 0

    // 代码段
    #.word 0xffff
    #.word 0
    #.byte 0
    #.byte 0b10011011
    #.byte 0b11001111
    #.byte 0

    // 数据段
    .word 0xffff
    .word 0
    .byte 0
    .byte 0b10010011
    .byte 0b11001111
    .byte 0

    // gdt 地址
.Ldata5:
    .word .Ldata5-.Ldata4-1
    .long .Ldata4

    // 内核32位代码加载位置
.Ldata6:
    .long 0x100000

.Ldata7:

    .fill 510-( . - .Lstart ), 1, 0
    .byte 0x55
    .byte 0xaa

.Lpart2:
    # 设置command line
    movw    $0x2000, %cx
    movw    %cx, %es
    movw    $.Ldata3, %si
    movw    $(.Ldata4-.Ldata3),%cx
    rep	movsb

    //set_header_fields:
    movw    $0x1000, %ax
    movw    %ax, %ds
    movb    $0xff, 0x210
    orb     $0x80, 0x211
    andb    $0xdf, 0x211
    movw    $0xfd55, 0x224
    #movb   $0x00, 0x226
    movb    $0x01, 0x227
    movl    $0x20000, 0x228

    //打开 A20
    movw    $0x2401, %ax
    int     $0x15
    jc      .Lerror
    testb   %ah, %ah
    jnz     .Lerror
    call    .Lclear


    //进入unreal模式
    cli
    lgdt    .Ldata5
    movl    %cr0, %eax
    orl     $1, %eax
    movl    %eax, %cr0
    jmp     .Lnext
.Lnext:
    movb    $8, %bl
    movw    %bx, %ds
    andb    $0xfe, %al
    movl    %eax, %cr0
    ljmpl   $0, $.Lnext2
.Lnext2:
    xorl    %eax, %eax
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    movw    %ax, %ds
    movw    %ax, %es
    movl    $0x7c00, %esp
    sti
    call    .Lclear

    //加载内核32位代码
    movl    %es:0x1f4, %eax
    movl    %eax, %ebx
    andl    $0x1f, %ebx
    jz      .Lnext3
    addl    $0x20, %eax
.Lnext3:
    shr     $5, %eax
.Lread_loop:
    pushl   %eax
    movw    $1, %ax
    movl    $0x30000000, %edx
    call    .Lread_hdd
    movl    .Ldata6, %eax
    addl    $0x200, .Ldata6
    movw    $0x3000, %bx
    movw    %bx, %es
    #xorl    %ecx, %ecx     #%ecx已经是0了
.Lmove_loop:
    movl    %es:(, %ecx, 4), %ebx
    movl    %ebx, (%eax, %ecx, 4)
    incb    %cl
    cmpb    $128, %cl
    jne     .Lmove_loop
    popl    %eax
    decl    %eax
    jnz     .Lread_loop


    //进入内核
    call    .Lclear
    cli
    movw    $0x1000, %ax
    movw    %ax, %ds
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    xorl    %eax, %eax
    #movl    $0xe000, %esp
    xorl    %esp, %esp
    ljmpl   $0x1020, $0x0
