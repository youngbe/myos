OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386:x86-64)
ENTRY(main)
SECTIONS
{
    .text 0x0 : {
        *(.text.startup)
        *(.text)
    }
    .rodata ALIGN(32) : { *(.rodata) }
    .data ALIGN(32) : {
        *(.data)
        . = ALIGN(32);
        *(.bss)
    }
    /* 当C语言使用__attribute__((constructor))或者c++在全局变量初始化时需要调用函数时，.init_array节就会出现 */
    /* 这部分的代码将在main函数之前执行 */
    .init_array : { *(.init_array) }
    ASSERT( SIZEOF(.init_array) == 0, ".init_array exist!" )
}
