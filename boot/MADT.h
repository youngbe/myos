#pragma once
#include <stdint.h>
#include <stddef.h>


#include "ACPI.h"

struct __attribute__((packed)) MADT
{
    struct ACPISDTHeader h;
    uint32_t local_apic_address;
    uint32_t flag;
};

struct __attribute__((packed)) Madt_entry_header
{
    uint8_t entry_type;
    uint8_t entry_length;
};

struct __attribute__((packed)) IO_APIC
{
    struct Madt_entry_header h;
    uint8_t id;
    uint8_t reserve;
    uint32_t address;
    uint32_t gsi_base;
};

struct __attribute__((packed)) Interrupt_Source_Override
{
    struct Madt_entry_header h;
    uint8_t bus;
    uint8_t irq;
    uint32_t gsi;
    uint16_t flag;
};

struct 
{
    ssize_t status;
    const struct MADT* madt_ptr;
}
get_madt( const struct XSDT* xsdt_ptr);

struct
{
    ssize_t status;
    size_t io_apic_ptr_list_size;
    size_t interrupt_source_override_ptr_list_size;
}
parse_madt(const struct MADT*madt,
        const struct IO_APIC** io_apic_ptr_list, size_t max_io_apic_ptr_list_size,
        const struct Interrupt_Source_Override** interrupt_source_override_ptr_list, size_t max_interrupt_source_override_ptr_list_size
        );

struct
{
    ssize_t status;
    uint32_t gsi;
} irq2gsi( uint8_t irq,
        const struct Interrupt_Source_Override** interrupt_source_override_ptr_list, size_t interrupt_source_override_ptr_list_size
        );

struct
{
    ssize_t status;
    const struct IO_APIC* io_apic_ptr;
} get_correspond_io_apic( uint32_t gsi, const struct IO_APIC** io_apic_ptr_list, size_t io_apic_ptr_list_size);
