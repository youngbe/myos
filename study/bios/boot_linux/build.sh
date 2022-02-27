#!/bin/bash -e
check_dependency()
{
    local temp=('set' 'as' 'dd' 'ld' 'rm' 'qemu-img' 'echo')
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
set -e
as --64 bootloader.s -o bootloader.o
ld --oformat binary -Ttext 0x7c00 -Tbss 0x0 -o bootloader.bin bootloader.o
dd conv=fdatasync if=bootloader.bin ibs=512 conv=sync of=boot.img
dd conv=fdatasync if=bzImage ibs=512 conv=sync of=boot.img oflag=append conv=notrunc
rm -rf boot.vmdk
qemu-img convert -f raw -O vmdk boot.img boot.vmdk
echo "==========================="
echo "生成boot.vmdk，可用于vmware"
echo "生成boot.img，可用于qemu"
