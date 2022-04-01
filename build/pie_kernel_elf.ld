OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
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
    .rela.dyn  ALIGN(48) :
    {
        __start_rela_dyn = . ;
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
        __stop_rela_dyn = . ;
    }

    .note.gnu.build-id  : { *(.note.gnu.build-id) }
    .hash           : { *(.hash) }
    .gnu.hash       : { *(.gnu.hash) }
    .dynsym         : { *(.dynsym) }
    .dynstr         : { *(.dynstr) }
    .dynamic        : { *(.dynamic) }

    /DISCARD/   :
    {
        *(.note)
        *(.comment .comment.*)
        *(.interp)
    }

    /*
  .rela.plt       :
    {
      *(.rela.plt)
      *(.rela.iplt)
    }
  .relr.dyn : { *(.relr.dyn) }
  .plt            : { *(.plt) *(.iplt) }
.plt.got        : { *(.plt.got) }
.plt.sec        : { *(.plt.sec) }
  .got            : { *(.got.plt) *(.igot.plt) *(.got) *(.igot) }
*/
    mytemp3 : { *(*) }
    ASSERT( SIZEOF(mytemp3) == 0, "unknown section exist!" )
}
