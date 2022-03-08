OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386:x86-64)
ENTRY(main)
SECTIONS
{
    .text : {
        *(.text.startup)
        *(.text)
    }
    . = ALIGN(16);
    .rodata : { *(.rodata) }
    . = ALIGN(16);
    .data : {
        *(.data)
        . = ALIGN(16);
        *(.bss)
    }
}
