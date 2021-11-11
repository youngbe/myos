#!/bin/bash
as --64 boot_loader.s -o boot_loader.o
ld --oformat binary -Ttext 0x7c00 -e 0x7c00 -o boot_loader.bin boot_loader.o
dd if=boot_loader.bin of=boot.img conv=sync,fdatasync
dd if=bzImage of=boot.img seek=2 conv=sync,fdatasync
rm -rf test.vmdk
qemu-img convert -f raw -O vmdk boot.img test.vmdk
