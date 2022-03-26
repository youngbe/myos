OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{
    myheap 0x0 (NOLOAD) : { *(myheap) }
    ASSERT( . == 0x7c00, "Bootloader heap size error!" )
    .text : {
        *(.text.startup)
        *(.text)
    }
    .rodata : { *(.rodata) }
    .data : {
        *(.data)
        *(.bss)
    }
    ASSERT( . <= 0x10000, "Bootloader too large!" )
}
