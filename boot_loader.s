    .code16
    .section .text
.Lstart:

    // 此Bootloader需要保证%ss %cs的值一直为0，直到进入linux内核为止
    // 初始化
    ljmp $0, $.Lreal_start
.Lreal_start:
    xorl    %eax, %eax
    cli
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
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









    //先读一个扇区
    movw    $1, %ax
    movl    $0x10000000, %edx
    call    .Lread_hdd

    /*
    movw    $0x0, %ax
    movw    %ax, %ds
    movl    .Ldata1, %ecx
    cmp     $2, %ecx
    jne     .Lerror
    */

    /*
    movw    $0x1000, %ax
    movw    %ax, %ds
    movw    %es:0x1f0, %ax
    cmp     $0x1bff, %ax
    jne     .Lerror
    */




    // 读取kernel setup sector
    movb    %es:0x1f1, %al

    // 检查是否太多扇区，不够空间存放
    cmpb    $0x3f, %al
    ja      .Lerror

    testb   %al, %al
    jne     .Lread_kernel_setup_next
    movb    $4, %al
.Lread_kernel_setup_next:
    movl    $0x10000200, %edx
    call    .Lread_hdd

    /*
    #movw    $0x0, %ax
    #movw    %ax, %ds
    #movl    .Ldata1, %ecx
    #cmp     $0x1d, %ecx
    #jne     .Lerror
    */

    /*
    #movw    $0x1000, %ax
    #movw    %ax, %ds
    #movw    0x5c0, %ax
    #cmp     $0xeb73, %ax
    #je     .Lerror
    */





    /*movw    $0x2401, %ax
    int     $0x15
    jc      .Lerror
    jmp     .Lerror
    */

    /*
    movw    $0, %ax
    movw    %ax, %ds
    movl    $0x107c00, %eax
    movb    (%eax), %al
    cmpb    $0, %al
    je      .Lerror
    */

    //set_header_fields:
    movw    $0x1000, %ax
    movw    %ax, %ds
    movb    $0xff, 0x210
    orb     $0x80, 0x211
    andb    $0xdf, 0x211
    movw    $0xde00, 0x224
    #movb   $0x00, 0x226
    movb    $0x01, 0x227
    movl    $0x1e000, 0x228

    //设置command line
    movl    $0x1000, %edi
    movw    %di, %es
    movw    $0xe000, %di
    xorw    %ax, %ax
    movw    %ax, %ds
    movw    $.Ldata3, %si
    movl    $(.Ldata4-.Ldata3),%ecx
    cld
    rep	movsb

    jmp     .Lerror

    //进入内核
    call    .Lclear
    cli
    movw    $0x1000, %ax
    movw    %ax, %ds
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    xorl    %eax, %eax
    movl    $0xe000, %esp
    ljmp    $0x1020, $0x0










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
    // 0号扇区是boot_loader，1号扇区开始是内核
.Ldata1:
    .long 1


.Ldata2:
    .ascii "error!"


    // cmd line
.Ldata3:
    .ascii "root=/dev/sda0\0"



.Ldata4:




    .fill 510-( . - .Lstart ), 1, 0
    .byte 0x55
    .byte 0xaa
