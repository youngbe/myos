启动Linux内核见study/bios/boot_linux

# myos
## 配置需求
1. 支持x86_64指令集
2. 以BIOS方式启动
3. 64位下支持1GB和2MB分页
4. 支持ACIP，且ACIP版本为2.0+
5. 支持x2APIC
## 接下来的计划
1. FPU初始化
2. 时钟中断和键盘中断初始化 (x2APIC)
