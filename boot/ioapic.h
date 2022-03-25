#include "libc.h"

static inline void write_ioapic_register(volatile uint32_t*const apic_base, const uint32_t offset, const uint32_t val)
{
    __asm__ volatile(
            "movl   %[offset], %[IOREGSEL]\n\t"
            "movl   %[val], %[IOWIN]"
            :[IOREGSEL]"=m"(*apic_base), [IOWIN]"=m"(*(apic_base+4))
            :[offset]"r"(offset), [val]"r"(val)
            :
            );
}

static inline uint32_t read_ioapic_register(volatile uint32_t*const apic_base, const uint32_t offset)
{
    uint32_t val;
    __asm__ volatile(
            "movl   %[offset], %[IOREGSEL]\n\t"
            "movl   %[IOWIN], %[val]"
            :[IOREGSEL]"=m"(*apic_base), [val]"=r"(val)
            :[offset]"r"(offset), [IOWIN]"m"(*(apic_base+4))
            :
            );
    return val;
}
