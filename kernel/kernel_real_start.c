
__attribute__((noreturn))
void kernel_real_start()
{
    //*(volatile char *)0xb9000='y';
    *(volatile char *)0xb8000='x';
    while (1);
    __builtin_unreachable();
}
