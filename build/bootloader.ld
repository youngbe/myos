OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{
    . = 0;
    .bss : { *(.bss) }
    . = 0x7c00;
    .text : {
        *(.text.startup)
        *(.text)
    }
    .rodata : { *(.rodata) }
    .data : { *(.data) }
    . = 0x10000;
}
