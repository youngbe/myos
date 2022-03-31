OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{
    mytemp1 : {
        KEEP (*(SORT_NONE(.fini)))
        KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
    }
    ASSERT( SIZEOF(mytemp1) == 0, "terminate code exist!" )

    mytemp2 : {
        KEEP (*(SORT_NONE(.init)))
        KEEP (*(.preinit_array))
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
    }
    ASSERT( SIZEOF(mytemp2) == 0, "init code exist!" )


    myheap 0x0 (NOLOAD) : { *(myheap) }
    ASSERT( . == 0x7c00, "Bootloader heap size error!" )
    .text : {
        *(.text.entry_point)
        *(.text.unlikely .text.*_unlikely .text.unlikely.*)
        *(.text.exit .text.exit.*)
        *(.text.startup .text.startup.*)
        *(.text.hot .text.hot.*)
        *(SORT(.text.sorted.*))
        *(.text .text.*)
    }
    ASSERT ( _start == 0x7c00 , "_start address error!" )
    .rodata ALIGN(32) :
    {
        *(.rodata .rodata.*) 
        /**(.gnu.linkonce.r.*)*/
    }
    .data ALIGN(32) : {
        *(.data .data.*)
        /**(.gnu.linkonce.d.*)*/
        SORT(CONSTRUCTORS)
        . = ALIGN(32);
        /**(.dynbss) */
        *(.bss .bss.*)
        /**(.gnu.linkonce.b.*)*/
        /**(COMMON) */
    }
    ASSERT( . <= 0x10000, "Bootloader too large!" )
    /DISCARD/   :
    {
        *(.note)
        *(.note.gnu.build-id)
        *(.hash)
        *(.gnu.hash)
        *(.dynsym)
        *(.dynstr)
        *(.dynamic)
        *(.comment .comment.*)
        *(.interp)
    }
    mytemp3 : { *(*) }
    ASSERT( SIZEOF(mytemp3) == 0, "unknown section exist!" )
}
