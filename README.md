启动Linux内核见study/bios/boot_linux

# myos
## 配置需求
1. 支持`x86-64-v3`指令集(x86_64,sse3,sse4.1,sse4.2,avx,avx2)
2. 支持以BIOS(legacy)方式启动，且Bootloader使用到了一些BIOS扩展(部分扩展qemu不支持)
3. 64位下支持1GB和2MB分页
4. 支持`ACIP`，且`ACIP`版本为2.0+(qemu版本过低)
5. 支持`x2APIC`(qemu不支持)
6. 支持`XSAVE feature set`

目前 vmware 16.2 + 第八代英特尔CPU 支持上述所有功能
## 开发环境
推荐在x86_64电脑上使用`Ubuntu 22.04+`或`Fedora 36+`

依赖安装：
```bash
# Ubuntu 22.04
sudo apt update
sudo apt --no-install-recommends install gcc g++ qemu-uitls
# Fedora
sudo dnf --setopt=install_weak_deps=0 install gcc gcc-c++ qemu-img
```
