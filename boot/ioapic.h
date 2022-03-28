#include "libc.h"

static inline void write_ioapic_register(void *const apic_base, const uint32_t offset, const uint32_t val)
{
    __asm__ volatile(
            "movl   %[offset], %[IOREGSEL]\n\t"
            "movl   %[val], %[IOWIN]"
            // 这里之所以使用"+m"而不是"=m"，是为了防止编译器优化write_ioapic_register和read_ioapic_register的执行顺序
            :[IOREGSEL]"+m"(*(uint32_t *)apic_base), [IOWIN]"+m"(*((uint32_t *)apic_base+4))
            :[offset]"ri"(offset), [val]"ri"(val)
            :
            );
}

static inline uint32_t read_ioapic_register(void*const apic_base, const uint32_t offset)
{
    uint32_t val;
    __asm__ volatile(
            "movl   %[offset], %[IOREGSEL]\n\t"
            "movl   %[IOWIN], %[val]"
            :[IOREGSEL]"+m"(*(uint32_t *)apic_base), [IOWIN]"+m"(*((uint32_t *)apic_base+4)), [val]"=r"(val)
            :[offset]"ri"(offset)
            :
            );
    return val;
}
