#include "libc.h"
#include "RSDP.h"
#include "ACPI.h"
#include "MADT.h"
#include "ioapic.h"

static inline struct
{
    ssize_t status;
    void* keyboard_ioapic_base;
    uint8_t index;
}
get_keyboard_ioapic_base_and_index();

ssize_t map_keyboard_interrupt_to_vector(const uint8_t apic_id, const uint8_t vector)
{
    void* keyboard_ioapic_base;
    uint8_t index_in_ioapic;
    {
        __auto_type ret=get_keyboard_ioapic_base_and_index();
        if ( ret.status != 0 )
        {
            return -1;
        }
        keyboard_ioapic_base=ret.keyboard_ioapic_base;
        index_in_ioapic=ret.index;
    }
    uint32_t keyboard_IOREDTBL_low=read_ioapic_register(keyboard_ioapic_base, (uint32_t)0x10+index_in_ioapic*2);
    uint32_t keyboard_IOREDTBL_high=read_ioapic_register(keyboard_ioapic_base, (uint32_t)0x10+index_in_ioapic*2+1);
    keyboard_IOREDTBL_low&= 0xFFFE5000;
    keyboard_IOREDTBL_low|=(uint32_t)vector;
    keyboard_IOREDTBL_high&=0x00ffffff;
    keyboard_IOREDTBL_high|=(uint32_t)apic_id<<24;
    write_ioapic_register(keyboard_ioapic_base, (uint32_t)0x10+index_in_ioapic*2, keyboard_IOREDTBL_low );
    write_ioapic_register(keyboard_ioapic_base, (uint32_t)0x10+index_in_ioapic*2+1, keyboard_IOREDTBL_high );
    return 0;
}

inline __typeof__(get_keyboard_ioapic_base_and_index()) get_keyboard_ioapic_base_and_index()
{
    __typeof__(get_keyboard_ioapic_base_and_index()) retv;
    const struct XSDT* xsdt_ptr;
    {
        __auto_type ret=find_RSDP();
        if ( ret.status != 0 )
        {
            retv.status=-1;
            return retv;
        }
        if ( ret.RSDP_ptr->firstPart.Revision != 2 )
        {
            retv.status=-2;
            return retv;
        }
        xsdt_ptr=(const struct XSDT*)ret.RSDP_ptr->XsdtAddress;
        if ( !valid_ACPISDT((const struct ACPISDTHeader*)xsdt_ptr) )
        {
            retv.status=-3;
            return retv;
        }
        const char temp[]={'X', 'S', 'D', 'T'};
        if ( strncmp(xsdt_ptr->h.Signature, temp, sizeof(temp)) != 0 )
        {
            retv.status=-4;
            return retv;
        }
    }

    // now get xsdt

    const struct MADT *madt_ptr;
    {
        __auto_type ret=get_madt(xsdt_ptr);
        if ( ret.status != 0 )
        {
            retv.status=-5;
            return retv;
        }
        madt_ptr=ret.madt_ptr;
    }

    // now get madt

    {
        const struct IO_APIC* io_apic_ptr_list[512];
        size_t io_apic_ptr_list_size;
        const struct Interrupt_Source_Override* interrupt_source_override_ptr_list[512];
        size_t interrupt_source_override_ptr_list_size;
        {
            __auto_type ret=parse_madt(madt_ptr, io_apic_ptr_list, 512, interrupt_source_override_ptr_list, 512);
            if ( ret.status != 0 )
            {
                retv.status=-6;
                return retv;
            }
            if ( ret.io_apic_ptr_list_size == 0 )
            {
                retv.status=-7;
                return retv;
            }
            io_apic_ptr_list_size=ret.io_apic_ptr_list_size;
            interrupt_source_override_ptr_list_size=ret.interrupt_source_override_ptr_list_size;
        }
        uint32_t keyboard_irq_gsi;
        {
            __auto_type ret=irq2gsi(1, interrupt_source_override_ptr_list, interrupt_source_override_ptr_list_size);
            if ( ret.status != 0 )
            {
                retv.status=-8;
                return retv;
            }
            keyboard_irq_gsi=ret.gsi;
        }
        {
            __auto_type ret=get_correspond_io_apic(keyboard_irq_gsi, io_apic_ptr_list, io_apic_ptr_list_size);
            if ( ret.status != 0 )
            {
                retv.status=-9;
                return retv;
            }
            retv.status=0;
            retv.keyboard_ioapic_base=(void *)(size_t)ret.io_apic_ptr->address;
            retv.index=(uint8_t)(keyboard_irq_gsi-ret.io_apic_ptr->gsi_base);
        }
    }
    return retv;
}
