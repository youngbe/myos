#!/bin/bash

CFLAGS_GLOBAL=("-std=c2x" "-g0" "-O3" "-Wall" "-Wextra" "-pedantic" "-fno-exceptions" "-fno-asynchronous-unwind-tables" "-fno-stack-check" "-fno-stack-clash-protection" "-fno-stack-protector" "-fno-unwind-tables" "-fcf-protection=none" "-fno-split-stack")

check_dependency()
{
    local temp=('set' 'as' 'dd' 'ld' 'rm' 'qemu-img' 'echo' 'gcc')
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
as --64 boot/bootloader.s -o out/bootloader.o
ld --oformat binary -Ttext 0x7c00 -Tbss 0x0 -o out/bootloader.bin out/bootloader.o

gcc "${CFLAGS_GLOBAL[@]}" -fpie -c kernel/main.c -o out/main.o
ld -T build/build.ld -pie out/main.o -o out/kernel.bin

gcc "${CFLAGS_GLOBAL[@]}" build/build_helper.c -o out/build_helper
out/build_helper out/kernel.bin out/kernel_size

dd conv=fdatasync if=out/bootloader.bin ibs=512 conv=sync of=out/boot.img
dd conv=fdatasync if=out/kernel_size ibs=512 conv=sync of=out/boot.img oflag=append conv=notrunc
dd conv=fdatasync if=out/kernel.bin ibs=512 conv=sync of=out/boot.img oflag=append conv=notrunc
rm -rf out/boot.vmdk
qemu-img convert -f raw -O vmdk out/boot.img out/boot.vmdk
