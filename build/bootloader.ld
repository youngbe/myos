OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{
    mytemp0 : {
        *(.text.startup.*)
    }
    ASSERT( SIZEOF(mytemp0) == 0, "unknown startup code!" )

    mytemp1 : {
        *(.fini)
        *(.fini_array)
        *(.text.exit)
    }
    ASSERT( SIZEOF(mytemp1) == 0, "terminate code exist!" )

    mytemp2 : {
        *(.pre_initarray)
        *(.init)
        *(.init_array)
    }
    ASSERT( SIZEOF(mytemp2) == 0, "init code exist!" )

    nouse_section (NOLOAD) : {
        *(.note .note.*)
        *(.comment .comment.*)
    }

    myheap 0x0 (NOLOAD) : { *(myheap) }
    ASSERT( . == 0x7c00, "Bootloader heap size error!" )
    .text : {
        *(.text.startup)
        *(.text .text.*)
    }
    .rodata : { *(.rodata .rodata.*) }
    .data : {
        *(.data .data.*)
        *(.bss .bss.*)
    }
    ASSERT( . <= 0x10000, "Bootloader too large!" )
    mytemp3 : { * }
    ASSERT( SIZEOF(mytemp3) == 0, "unknown section exist!" )
}
