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
as --64 boot_loader.s -o boot_loader.o
ld --oformat binary --section-start start=0x0 -Ttext 0x7c00 -o boot_loader.bin boot_loader.o
dd if=boot_loader.bin of=boot.img conv=sync,fdatasync
rm -rf boot.vmdk
qemu-img convert -f raw -O vmdk boot.img boot.vmdk
echo "==========================="
echo "生成boot.vmdk，可用于vmware"
echo "生成boot.img，可用于qemu"
