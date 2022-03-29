#include "libc.h"

static inline void write_ioapic_register(void *const apic_base, const uint32_t offset, const uint32_t val)
{
    __asm__ volatile(
            "movl   %[offset], %[IOREGSEL]\n\t"
            "movl   %[val], %[IOWIN]"
            :[IOREGSEL]"=m"(((volatile uint32_t *)apic_base)[0]), [IOWIN]"=m"(((volatile uint32_t *)apic_base)[4])
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
            :[IOREGSEL]"=m"(((volatile uint32_t *)apic_base)[0]), [IOWIN]"=m"(((volatile uint32_t *)apic_base)[4]), [val]"=r"(val)
            :[offset]"ri"(offset)
            :
            );
    return val;
}
