#include <stddef.h>
#include <string.h>

#include "MADT.h"
#include "ioapic.h"

// ret.status: -1:未找到，-2:找到多个
__typeof__(get_madt(NULL)) get_madt(const struct XSDT* xsdt_ptr)
{
    __typeof__(get_madt(NULL)) ret;
    ret.status=-1;
    const char temp[4]={'A', 'P', 'I', 'C'};
    for ( size_t i=0, sum=(xsdt_ptr->h.Length-sizeof(struct ACPISDTHeader)) /sizeof(uint64_t); i<sum ; ++i )
    {
        if ( valid_ACPISDT( (const struct ACPISDTHeader *)xsdt_ptr->PointerToOtherSDT[i]) )
        {
            if ( strncmp( ((const struct ACPISDTHeader *)xsdt_ptr->PointerToOtherSDT[i])->Signature, temp, sizeof(temp) ) == 0 )
            {
                if ( ret.status == -1 )
                {
                    ret.status=0;
                    ret.madt_ptr=(const struct MADT *)xsdt_ptr->PointerToOtherSDT[i];
                }
                else
                {
                    ret.status=-2;
                    return ret;
                }
            }
        }
    }
    return ret;
}

// ret.status: -2：长度不符，-1：未知类型，-3：空间不足存放
__typeof__(parse_madt(NULL,NULL,0,NULL,0)) parse_madt(
        const struct MADT*madt,
        const struct IO_APIC** io_apic_ptr_list, size_t max_io_apic_ptr_list_size,
        const struct Interrupt_Source_Override** interrupt_source_override_ptr_list, size_t max_interrupt_source_override_ptr_list_size
        )
{
    __typeof__(parse_madt(NULL,NULL,0,NULL,0)) ret;
    ret.io_apic_ptr_list_size=0;
    ret.interrupt_source_override_ptr_list_size=0;
    const struct Madt_entry_header* temp_ptr=(const struct Madt_entry_header*)((const uint8_t *)madt+0x2c);
    while ( (const uint8_t *)temp_ptr < (const  uint8_t *)madt+madt->h.Length )
    {
        // Entry Type 0 : Processor Local APIC
        if ( temp_ptr->entry_type == 0 )
        {
            if ( temp_ptr->entry_length != 8 )
            {
                ret.status=-2;
                return ret;
            }
        }
        // Entry Type 1 : I/O APIC
        else if ( temp_ptr->entry_type == 1 )
        {
            if ( temp_ptr->entry_length != 12 )
            {
                ret.status=-2;
                return ret;
            }
            if ( ret.io_apic_ptr_list_size == max_io_apic_ptr_list_size )
            {
                ret.status=-3;
                return ret;
            }
            io_apic_ptr_list[ret.io_apic_ptr_list_size++]=(const struct IO_APIC *)temp_ptr;
        }
        // Entry Type 2 : IO/APIC Interrupt Source Override
        else if ( temp_ptr->entry_type == 2 )
        {
            if ( temp_ptr->entry_length != 10 )
            {
                ret.status=-2;
                return ret;
            }
            if ( ret.interrupt_source_override_ptr_list_size == max_interrupt_source_override_ptr_list_size )
            {
                ret.status=-3;
                return ret;
            }
            interrupt_source_override_ptr_list[ret.interrupt_source_override_ptr_list_size++]=(const struct Interrupt_Source_Override *)temp_ptr;
        }
        // Entry type 3 : IO/APIC Non-maskable interrupt source
        else if ( temp_ptr->entry_type == 3 )
        {
            if ( temp_ptr->entry_length != 10 )
            {
                ret.status=-2;
                return ret;
            }
        }
        // Entry Type 4 : Local APIC Non-maskable interrupts
        else if ( temp_ptr->entry_type == 4 )
        {
            if ( temp_ptr->entry_length != 6 )
            {
                ret.status=-2;
                return ret;
            }
        }
        // Entry Type 5 : Local APIC Address Override
        else if ( temp_ptr->entry_type == 5 )
        {
            if ( temp_ptr->entry_length != 12 )
            {
                ret.status=-2;
                return ret;
            }
        }
        // Entry Type 9 : Processor Local x2APIC
        else if ( temp_ptr->entry_type == 9 )
        {
            if ( temp_ptr->entry_length != 16 )
            {
                ret.status=-2;
                return ret;
            }
        }
        // unknown Entry Type
        else
        {
            ret.status=-1;
            return ret;
        }
        temp_ptr= (const struct Madt_entry_header*)((const uint8_t *)temp_ptr+temp_ptr->entry_length);
    }
    ret.status=0;
    return ret;
}

__typeof__(irq2gsi(0,NULL,0)) irq2gsi( uint8_t irq,
        const struct Interrupt_Source_Override** interrupt_source_override_ptr_list, size_t interrupt_source_override_ptr_list_size
        )
{
    __typeof__(irq2gsi(0,NULL,0)) ret={0, irq};
    bool find=false;
    for ( size_t i=0; i<interrupt_source_override_ptr_list_size; ++i )
    {
        if ( interrupt_source_override_ptr_list[i]->irq == irq )
        {
            if ( find )
            {
                ret.status=-1;
                return ret;
            }
            find=true;
            ret.gsi=interrupt_source_override_ptr_list[i]->gsi;
        }
    }
    return ret;
}

// ret.status == -1 ：未发现， -2：发现多个
__typeof__(get_correspond_io_apic(0, NULL, 0)) get_correspond_io_apic(uint32_t gsi, const struct IO_APIC** io_apic_ptr_list, size_t io_apic_ptr_list_size)
{
    __typeof__(get_correspond_io_apic(0, NULL, 0)) ret;
    ret.status=-1;
    for ( size_t i=0; i<io_apic_ptr_list_size; ++i )
    {
        if ( io_apic_ptr_list[i]->gsi_base<=gsi )
        {
            if ( (( read_ioapic_register((void *)(size_t)io_apic_ptr_list[i]->address, 1) >>16)&0xff) + io_apic_ptr_list[i]->gsi_base >=gsi )
            {
                if ( ret.status == -1 )
                {
                    ret.status=0;
                    ret.io_apic_ptr=io_apic_ptr_list[i];
                }
                else
                {
                    ret.status=-2;
                    return ret;
                }
            }
        }
    }
    return ret;
}
