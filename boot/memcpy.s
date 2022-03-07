    .code64
    .seection .text
    .p2align 4
    .globl  memcpy
memcpy:
    cld
    movq    %rdx, %rcx
    shrq    $3, %rcx
    rep;    movsq
    movq    %rdx, %rcx
    addq    $7, %rcx
    rep;    movsb
    ret
