OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
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

    .rela.dyn   :
    {
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
    ASSERT( SIZEOF(.rela.dyn) == 0 , ".rela.dyn exist!" )

    /DISCARD/   :
    {
        *(.note .note.*)
        *(.comment .comment.*)
        *(.interp)
    }

    ASSERT( . == 0 , "ld script error!" )

    .text ALIGN(32) : {
        *(.text.startup)
        *(.text.hot .text.hot.*)
        *(SORT(.text.sorted.*))
        *(.text .text.*)
        *(.text.unlikely .text.*_unlikely .text.unlikely.*)
    }
    .rodata ALIGN(32) : { *(.rodata .rodata.*) }
    .data ALIGN(32) : {
        *(.data .data.*)
        . = ALIGN(32);
        *(.bss .bss.*)
    }

    .hash           : { *(.hash) }
    .gnu.hash       : { *(.gnu.hash) }
    .dynsym         : { *(.dynsym) }
    .dynstr         : { *(.dynstr) }
    .dynamic        : { *(.dynamic) }

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
    mytemp3 : { * }
    ASSERT( SIZEOF(mytemp3) == 0, "unknown section exist!" )
}
