启动Linux内核见study/bios/boot_linux

# myos
## 配置需求
1. 支持x86_64指令集
2. 以BIOS(legacy)方式启动
3. 支持BIOS拓展(部分拓展qemu不支持)
4. 64位下支持1GB和2MB分页
5. 支持ACIP，且ACIP版本为2.0+(qemu版本过低)
6. 支持x2APIC(qemu不支持)

目前 vmware 16.2 支持上述功能
## 接下来的计划
1. FPU初始化
2. 时钟中断和键盘中断初始化 (x2APIC)
