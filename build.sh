#!/bin/bash

# GCC 通用 CFLAGS ，主要是开一些优化，关闭一些安全机制提升程序效率
GCC_GLOBAL_CFLAGS=("-std=c2x" "-g0" "-O3" "-Wall" "-Wextra" \
    "-fstack-reuse=all" "-freg-struct-return" "-fdwarf2-cfi-asm" "-fplt" \
    "-fwrapv" "-fwrapv-pointer" "-fno-trapv" \
    "-fno-exceptions" "-fno-asynchronous-unwind-tables" "-fno-unwind-tables" \
    "-fstack-check=no" "-fno-stack-clash-protection" "-fno-stack-protector" "-fno-split-stack" "-fcf-protection=none" "-fno-sanitize=all" "-fno-instrument-functions")

# 开启LTO优化的FLAGS
LTO_FLAGS=("-flto" "-flto-compression-level=0" "-fno-fat-lto-objects" "-fuse-linker-plugin" "-fwhole-program")

# 加上这个 FLAGS 将移除所有库，编译纯C程序
PURE_C_FLAGS=("-fno-builtin" "-nostdinc" "-nostdlib" "-nolibc" "-nostartfiles" "-nodefaultlibs")

KERNEL_CFLAGS=("${PURE_C_FLAGS[@]}" "-ffreestanding" "-mno-red-zone" "-mgeneral-regs-only")

PIE_KERNEL_ELF_OUTPUT_FLAGS=("${KERNEL_CFLAGS[@]}" "-fpie" "-T" "build/pie_kernel_elf.ld" "-pie" "build/kernel_start.c")

BOOTLOADER_BIN_OUTPUT_FLAGS=("${PURE_C_FLAGS[@]}" "-mno-red-zone" "-mgeneral-regs-only" "-fno-pie" "-T" "build/bootloader.ld" "-no-pie")

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
$CC "${GCC_GLOBAL_CFLAGS[@]}" "${BOOTLOADER_BIN_OUTPUT_FLAGS[@]}" \
    -m32 -S \
    -I libc/include -I include \
    boot/handle_memory_map.c \
    -o out/handle_memory_map.s
echo "  .code32" | cat - out/handle_memory_map.s > out/handle_memory_map.s.new
mv out/handle_memory_map.s.new out/handle_memory_map.s

$CC "${GCC_GLOBAL_CFLAGS[@]}" "${LTO_FLAGS[@]}" "${BOOTLOADER_BIN_OUTPUT_FLAGS[@]}" \
    -I libc/include -I include libc/memcpy32.s libc/memset32.s libc/memmove32.s libc/memcmp32.s \
    boot/bootloader.s out/handle_memory_map.s boot/RSDP.c boot/MADT.c boot/init_ioapic_keyboard.c boot/error.c \
    -o out/bootloader.bin


$CC "${GCC_GLOBAL_CFLAGS[@]}" "${LTO_FLAGS[@]}" "${PIE_KERNEL_ELF_OUTPUT_FLAGS[@]}" \
    -I kernel/include -I libc/include -I include libc/memcpy.s libc/memset.s libc/memmove.s libc/memcmp.s \
    kernel/main4.c kernel/kernel_real_start.c kernel/pages.c \
    -o out/kernel.elf
$OBJCOPY -O binary -j .text --set-section-flags .text=load,content,alloc \
    -j .rodata --set-section-flags .rodata=load,content,alloc \
    -j .data --set-section-flags .data=load,content,alloc \
    -j .bss --set-section-flags .bss=load,content,alloc \
    -j .rela.dyn --set-section-flags .rela.dyn=load,content,alloc \
    out/kernel.elf out/kernel.bin


$HOSTCC "${GCC_GLOBAL_CFLAGS[@]}" "${LTO_FLAGS[@]}" build/build_helper.c -o out/build_helper
out/build_helper out/kernel.bin out/kernel_size

dd conv=fdatasync if=out/bootloader.bin ibs=$((512*65)) conv=sync of=out/boot.img
dd conv=fdatasync if=out/kernel_size ibs=512 conv=sync of=out/boot.img oflag=append conv=notrunc
dd conv=fdatasync if=out/kernel.bin ibs=512 conv=sync of=out/boot.img oflag=append conv=notrunc
dd conv=fdatasync if=/dev/zero ibs=1M count=2 of=out/boot.img oflag=append conv=notrunc
rm -rf out/boot.vmdk
qemu-img convert -f raw -O vmdk out/boot.img out/boot.vmdk
