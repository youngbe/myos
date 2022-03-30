OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386:x86-64)
ENTRY(main)
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

    .text 0x0 : {
        *(.text.startup)
        *(.text .text.*)
    }
    .rodata ALIGN(32) : { *(.rodata .rodata.*) }
    .data ALIGN(32) : {
        *(.data .data.*)
        . = ALIGN(32);
        *(.bss .bss.*)
    }
    
    mytemp3 : { * }
    ASSERT( SIZEOF(mytemp3) == 0, "unknown section exist!" )
}
