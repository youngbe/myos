#!/bin/bash
DEBUG=0
for ((i=0;i<$#;++i))
do
    if [ "${@:$i+1:1}" == "--debug" ]; then
        DEBUG=1
    fi
done

# GCC 通用 CFLAGS ，不仅可以用于编译内核，也可以用于编译普通应用程序
# 包括开启一些优化，关闭一些安全机制提升程序效率
# 以及一些个人习惯使用的选项
GCC_GLOBAL_CFLAGS=("-std=c2x" "-g0" "-O3" "-Wall" "-Wextra" \
    "-fstack-reuse=all" "-freg-struct-return" "-fdwarf2-cfi-asm" "-fplt" \
    "-fwrapv" "-fwrapv-pointer" "-fno-trapv" \
    "-fno-exceptions" "-fno-asynchronous-unwind-tables" "-fno-unwind-tables" \
    "-fstack-check=no" "-fno-stack-clash-protection" "-fno-stack-protector" "-fno-split-stack" "-fcf-protection=none" "-fno-sanitize=all" "-fno-instrument-functions")

# GCC 通用 C++ FLAGS，同上
GCC_GLOBAL_CXXFLAGS=("-std=c++23" "-g0" "-O3" "-Wall" "-Wextra" \
    "-fstack-reuse=all" "-freg-struct-return" "-fdwarf2-cfi-asm" "-fplt" \
    "-fwrapv" "-fwrapv-pointer" "-fno-trapv" \
    "-fno-rtti" "-fno-threadsafe-statics" \
    "-fstack-check=no" "-fno-stack-clash-protection" "-fno-stack-protector" "-fno-split-stack" "-fcf-protection=none" "-fno-sanitize=all" "-fno-instrument-functions" "-fvtable-verify=none")

# 开启LTO优化的FLAGS
LTO_FLAGS=("-flto" "-flto-compression-level=0" "-fno-fat-lto-objects" "-fuse-linker-plugin" "-fwhole-program")

# 加上这个 FLAGS 将移除所有标准库，编译纯C/C++程序
PURE_FLAGS=("-fno-builtin" "-nostdinc" "-nostdlib" "-nolibc" "-nostartfiles" "-nodefaultlibs")

if [ -z "$CC" ]; then
    CC="x86_64-linux-gnu-gcc"
fi
if [ -z "$CXX" ]; then
    CXX="x86_64-linux-gnu-g++"
fi
if [ -z "$HOSTCC" ]; then
    HOSTCC="gcc"
fi
if [ -z "$AS" ]; then
    AS="x86_64-linux-gnu-as"
fi
if [ -z "$LD" ]; then
    LD="x86_64-linux-gnu-ld"
fi
if [ -z "$OBJCOPY" ]; then
    OBJCOPY="x86_64-linux-gnu-objcopy"
fi

check_dependency()
{
    local temp=('set' "$AS" 'dd' "$LD" 'rm' 'qemu-img' 'echo' "$CC" "$HOSTCC")
    for i in "${temp[@]}"
    do
        if ! command -V "$i" >/dev/null 2>&1; then
            echo " \"${i}\" 命令未找到"
            echo "请检查依赖是否正确安装"
            exit 1
        fi
    done
}
check_dependency
mkdir out 2>/dev/null
set -e
set -o xtrace
$CC "${GCC_GLOBAL_CFLAGS[@]}" \
    "${PURE_FLAGS[@]}" -mno-red-zone -mgeneral-regs-only -fno-pie -m32 -S \
    -I libc/include -I include \
    bootloader/process_memory_map.c \
    -o out/process_memory_map.s
echo "  .code32" | cat - out/process_memory_map.s > out/process_memory_map.s.new
mv out/process_memory_map.s.new out/process_memory_map.s

$CC "${GCC_GLOBAL_CFLAGS[@]}" "${LTO_FLAGS[@]}" \
    "${PURE_FLAGS[@]}" -mno-red-zone -mgeneral-regs-only -fno-pie -T bootloader/bootloader_bin.ld -no-pie \
    -I libc/include -I include libc/memcpy32.s libc/memset32.s libc/memmove32.s libc/memcmp32.s \
    bootloader/bootloader.s out/process_memory_map.s bootloader/RSDP.c bootloader/MADT.c bootloader/init_ioapic_keyboard.c bootloader/error.c \
    -o out/bootloader.bin


if [ $DEBUG -eq 0 ]; then
    # 目前bootloader还没有开启浮点功能，开启后将删除-mgeneral-regs-only
    $CXX "${GCC_GLOBAL_CXXFLAGS[@]}" "${LTO_FLAGS[@]}" \
        "${PURE_FLAGS[@]}" -ffreestanding -mno-red-zone -fpie -T kernel_test/kernel_pie_elf.ld -pie -fno-use-cxa-atexit -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables \
        -mgeneral-regs-only \
        -I libc/include -I include libc/memcpy.s libc/memset.s libc/memmove.s libc/memcmp.s \
        kernel_test/_start.cpp kernel_test/init.cpp kernel_test/terminal.cpp kernel_test/test.cpp \
        -o out/kernel.elf
else
    echo "" > out/empty.s
    $AS --64 out/empty.s -o out/empty.o
    $LD -s -e 0 out/empty.o -o out/empty.elf
fi
$OBJCOPY -O binary -j .text --set-section-flags .text=load,content,alloc \
    -j .rodata --set-section-flags .rodata=load,content,alloc \
    -j .data --set-section-flags .data=load,content,alloc \
    -j .bss --set-section-flags .bss=load,content,alloc \
    -j .rela.dyn --set-section-flags .rela.dyn=load,content,alloc \
    -j .init_array --set-section-flags .init_array=load,content,alloc \
    -j .fini_array --set-section-flags .fini_array=load,content,alloc \
    out/kernel.elf out/kernel.bin


$HOSTCC "${GCC_GLOBAL_CFLAGS[@]}" "${LTO_FLAGS[@]}" build/build_helper.c -o out/build_helper
out/build_helper out/kernel.bin out/kernel_size

dd conv=fdatasync if=out/bootloader.bin ibs=$((512*65)) conv=sync of=out/boot.img
dd conv=fdatasync if=out/kernel_size ibs=512 conv=sync of=out/boot.img oflag=append conv=notrunc
dd conv=fdatasync if=out/kernel.bin ibs=512 conv=sync of=out/boot.img oflag=append conv=notrunc
dd conv=fdatasync if=/dev/zero ibs=1M count=2 of=out/boot.img oflag=append conv=notrunc
rm -rf out/boot.vmdk
qemu-img convert -f raw -O vmdk out/boot.img out/boot.vmdk
