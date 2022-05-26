OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS
{
    mytemp1 : {
        KEEP (*(SORT_NONE(.fini)))
    }
    ASSERT( SIZEOF(mytemp1) == 0, "unsupported terminate code exist!" )

    mytemp2 : {
        KEEP (*(SORT_NONE(.init)))
        KEEP (*(.preinit_array))
    }
    ASSERT( SIZEOF(mytemp2) == 0, "unsupported init code exist!" )

    ASSERT( . == 0 , "ld script error!" )

    .text ALIGN(32) : {
        *(.text.entry_point)
        *(.text.unlikely .text.*_unlikely .text.unlikely.*)
        *(.text.exit .text.exit.*)
        *(.text.startup .text.startup.*)
        *(.text.hot .text.hot.*)
        *(SORT(.text.sorted.*))
        *(.text .text.*)
    }
    ASSERT ( _start == 0, "_start address error!" )
    .rodata ALIGN(32) :
    {
        *(.rodata .rodata.*) 
        /**(.gnu.linkonce.r.*)*/
    }
    .data ALIGN(32) : {
        *(.data .data.*)
        /**(.gnu.linkonce.d.*)*/
        SORT(CONSTRUCTORS)
    }
    .bss ALIGN(32) :
    {
        /**(.dynbss) */
        *(.bss .bss.*)
        /**(.gnu.linkonce.b.*)*/
        /**(COMMON) */
    }
    __kernel_end = . ;
    .init_array ALIGN(8) :
    {
        /*PROVIDE_HIDDEN (__init_array_start = .);*/
        __init_array_start = .;
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
        /*PROVIDE_HIDDEN (__init_array_end = .);*/
    }
    __init_array_end = .;
    .fini_array ALIGN(8) :
    {
        /*PROVIDE_HIDDEN (__fini_array_start = .);*/
        __fini_array_start = .;
        KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
        /*PROVIDE_HIDDEN (__fini_array_end = .);*/
    }
    __fini_array_end = .;
    .rela.dyn  ALIGN(48) :
    {
        __rela_dyn_start = . ;
        *(.rela.init)
        *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*)
        *(.rela.fini)
        *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*)
        *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*)
        *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*)
        *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*)
        *(.rela.ctors)
        *(.rela.dtors)
        *(.rela.got)
        *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)
        *(.rela.ldata .rela.ldata.* .rela.gnu.linkonce.l.*)
        *(.rela.lbss .rela.lbss.* .rela.gnu.linkonce.lb.*)
        *(.rela.lrodata .rela.lrodata.* .rela.gnu.linkonce.lr.*)
        *(.rela.ifunc)
    }
    __rela_dyn_end = . ;

    /* 一些没有影响的信息 */
    .note.gnu.build-id  : { *(.note.gnu.build-id) }
    .note.gnu.property  : { *(.note.gnu.property) }
    .hash           : { *(.hash) }
    .gnu.hash       : { *(.gnu.hash) }
    .dynsym         : { *(.dynsym) }
    .dynstr         : { *(.dynstr) }
    .dynamic        : { *(.dynamic) }

    /* 调试信息 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions.  */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2.  */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2.  */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions.  */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
    /* DWARF 3.  */
    .debug_pubtypes 0 : { *(.debug_pubtypes) }
    .debug_ranges   0 : { *(.debug_ranges) }
    /* DWARF 5.  */
    .debug_addr     0 : { *(.debug_addr) }
    .debug_line_str 0 : { *(.debug_line_str) }
    .debug_loclists 0 : { *(.debug_loclists) }
    .debug_macro    0 : { *(.debug_macro) }
    .debug_names    0 : { *(.debug_names) }
    .debug_rnglists 0 : { *(.debug_rnglists) }
    .debug_str_offsets 0 : { *(.debug_str_offsets) }
    .debug_sup      0 : { *(.debug_sup) }
    .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
    /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }

    /DISCARD/   :
    {
        *(.note)
        *(.note.*)
        *(.comment .comment.*)
        *(.interp)
    }

    /*mytemp3 : { *(*) }*/
    /*ASSERT( SIZEOF(mytemp3) == 0, "unknown section exist!" )*/
}
