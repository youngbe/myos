#!/bin/bash

# GCC 通用 CFLAGS ，主要是开一些优化，关闭一些安全机制提升程序效率
GCC_GLOBAL_CFLAGS=("-std=c2x" "-g0" "-O3" "-Wall" "-Wextra" "-pedantic" \
    "-fstack-reuse=all" "-freg-struct-return" "-fdwarf2-cfi-asm" \
    "-fwrapv" "-fwrapv-pointer" "-fno-trapv" \
    "-fno-exceptions" "-fno-asynchronous-unwind-tables" "-fno-unwind-tables" \
    "-fno-stack-check" "-fno-stack-clash-protection" "-fno-stack-protector" "-fno-split-stack" "-fcf-protection=none" "-fno-sanitize=all" "-fno-instrument-functions")

# 开启LTO优化的FLAGS
LTO_FLAGS=("-flto" "-flto-compression-level=0" "-fno-fat-lto-objects" "-fuse-linker-plugin" "-fwhole-program")

# 加上这个 FLAGS 可以编译出纯二进制的可重定位可执行程序
# 可以加载到内存任意位置然后直接跳转过去就能执行
PIE_BINARY_FLAGS=("-fpie" "-pie" "-T" "build/pie_binary.ld")

# 加上这个 FLAGS 将移除所有库，编译纯C程序
PURE_C_FLAGS=("-fno-builtin" "-ffreestanding" "-nostdinc" "-nostdlib")

if [ -z "$CC" ]; then
    CC="x86_64-linux-gnu-gcc"
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
$AS --64 boot/bootloader.s -o out/bootloader.o
$LD --oformat binary -Ttext 0x7c00 -Tbss 0x0 -o out/bootloader.bin out/bootloader.o


$CC "${GCC_GLOBAL_CFLAGS[@]}" "${LTO_FLAGS[@]}" "${PURE_C_FLAGS[@]}" "${PIE_BINARY_FLAGS[@]}" \
    -I include/public -I include/private \
    kernel/main.c kernel/terminal_io.c \
    -o out/kernel.bin
#ld -T build/build.ld -pie out/main.o -o out/kernel.bin

$HOSTCC "${GCC_GLOBAL_CFLAGS[@]}" "${LTO_FLAGS[@]}" build/build_helper.c -o out/build_helper
out/build_helper out/kernel.bin out/kernel_size

dd conv=fdatasync if=out/bootloader.bin ibs=512 conv=sync of=out/boot.img
dd conv=fdatasync if=out/kernel_size ibs=512 conv=sync of=out/boot.img oflag=append conv=notrunc
dd conv=fdatasync if=out/kernel.bin ibs=512 conv=sync of=out/boot.img oflag=append conv=notrunc
rm -rf out/boot.vmdk
qemu-img convert -f raw -O vmdk out/boot.img out/boot.vmdk
