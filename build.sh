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
GCC_GLOBAL_CFLAGS=(
    -std=c2x -Wall -Wextra
    -fstack-reuse=all -freg-struct-return -fdwarf2-cfi-asm -fplt -fipa-pta -fdevirtualize-at-ltrans
    -fwrapv -fwrapv-pointer -fno-trapv -ffast-math -ffp-contract=fast
    -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables
    -fstack-check=no -fno-stack-clash-protection -fno-stack-protector -fno-split-stack -fcf-protection=none -fno-sanitize=all -fno-instrument-functions
    -g0 -Ofast
)

# GCC 通用 C++ FLAGS，同上
GCC_GLOBAL_CXXFLAGS=(
    -std=c++23 -Wall -Wextra
    -fstack-reuse=all -freg-struct-return -fdwarf2-cfi-asm -fplt -fipa-pta -fdevirtualize-at-ltrans
    -fwrapv -fwrapv-pointer -fno-trapv -ffast-math -ffp-contract=fast -fno-rtti -fno-threadsafe-statics
    -fstack-check=no -fno-stack-clash-protection -fno-stack-protector -fno-split-stack -fcf-protection=none -fno-sanitize=all -fno-instrument-functions -fvtable-verify=none
    -g0 -Ofast
)

# 开启LTO优化的FLAGS
LTO_FLAGS=(-flto -flto-partition=none -flto-compression-level=0 -fno-fat-lto-objects -fuse-linker-plugin -fwhole-program)

# 加上这个 FLAGS 将移除所有标准库，编译纯C/C++程序
# 加上-fno-builtin时会默认关闭-ftree-loop-distribute-patterns
# 见：https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56888
PURE_FLAGS=(-fno-builtin -ftree-loop-distribute-patterns -nostdinc -nostdlib -nolibc -nostartfiles -nodefaultlibs)

if [ -z "$CC" ]; then
    CC="gcc"
fi
if [ -z "$CXX" ]; then
    CXX="g++"
fi
if [ -z "$HOSTCC" ]; then
    HOSTCC="gcc"
fi
if [ -z "$AS" ]; then
    AS="as"
fi
if [ -z "$LD" ]; then
    LD="ld"
fi
if [ -z "$OBJCOPY" ]; then
    OBJCOPY="objcopy"
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

# 首先编译内核
if [ $DEBUG -eq 0 ]; then
    # 开启-ffreestanding后将默认关闭-ftree-loop-distribute-patterns
    # 见：https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56888
    $CXX "${GCC_GLOBAL_CXXFLAGS[@]}" "${LTO_FLAGS[@]}" \
        -march=x86-64-v3 \
        "${PURE_FLAGS[@]}" -ffreestanding -ftree-loop-distribute-patterns -mno-red-zone -fpie -T kernel_test/kernel_pie_elf.ld -pie -fno-use-cxa-atexit -fno-exceptions -fno-asynchronous-unwind-tables -fno-unwind-tables \
	-static-pie -s -Wl,--build-id=none \
        -I libc/include -I include libc/memcpy.s libc/memset.s libc/memmove.s libc/memcmp.s \
        kernel_test/_start.cpp kernel_test/init.cpp kernel_test/kernel_start.cpp kernel_test/empty_isr.s kernel_test/sched/return_handlers.s kernel_test/sched/timer_isr.s \
        -o out/kernel.elf
else
    > out/empty.s
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

# 然后编译bootloader
sectors_num=$(( $(stat -c %s out/kernel.bin) ))
if ((sectors_num%512 == 0)) ; then
    sectors_num=$((sectors_num/512))
else
    sectors_num=$((sectors_num/512+1))
fi
sed "s/__pp_kernel_sectors_num/$sectors_num/g" bootloader/bootloader.s > out/bootloader.s

$CC "${GCC_GLOBAL_CFLAGS[@]}" \
    "${PURE_FLAGS[@]}" -mno-red-zone -fno-pie -m32 -S \
    -I libc/include -I include \
    bootloader/process_memory_map.c \
    -o out/process_memory_map.s
echo "  .code32" | cat - out/process_memory_map.s > out/process_memory_map.s.new
mv out/process_memory_map.s.new out/process_memory_map.s

$CC "${GCC_GLOBAL_CFLAGS[@]}" "${LTO_FLAGS[@]}" \
    "${PURE_FLAGS[@]}" -mno-red-zone -fno-pie -T bootloader/bootloader_bin.ld -no-pie \
    -I libc/include -I include libc/memcpy32.s libc/memset32.s libc/memmove32.s libc/memcmp32.s \
    out/bootloader.s out/process_memory_map.s bootloader/RSDP.c bootloader/MADT.c bootloader/init_ioapic_keyboard.c bootloader/error.c \
    -o out/bootloader.bin

# 最后将内核和bootloader打包为虚拟磁盘
dd conv=fdatasync if=out/bootloader.bin ibs=$((512*66)) conv=sync of=out/boot.img
dd conv=fdatasync if=out/kernel.bin ibs=512 conv=sync of=out/boot.img oflag=append conv=notrunc
dd conv=fdatasync if=/dev/zero ibs=1M count=2 of=out/boot.img oflag=append conv=notrunc
rm -rf out/boot.vmdk
qemu-img convert -f raw -O vmdk out/boot.img out/boot.vmdk
