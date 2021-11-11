#!/bin/bash
if [ "$1" == "" ]; then
    as --64 boot_loader.s -o boot_loader.o
else
    as --64 "$1" -o boot_loader.o
fi
ld --oformat binary -Ttext 0x7c00 -e 0x7c00 -o boot_loader.bin boot_loader.o
> boot.img
dd if=boot_loader.bin of=boot.img conv=notrunc
dd if=bzImage of=boot.img seek=2 conv=notrunc
qemu-img convert -f raw -O vmdk boot.img test.vmdk
