#pragma once
#include <stdint.h>
#include <stdbool.h>

struct ACPISDTHeader
{
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
} __attribute__ ((packed));

struct XSDT
{
    struct ACPISDTHeader h;
    uint64_t PointerToOtherSDT[];
} __attribute__ ((packed));

static inline bool valid_ACPISDT(const struct ACPISDTHeader* ptr);

inline bool valid_ACPISDT( const struct ACPISDTHeader* ptr)
{
    if ( ptr->Length == 0 )
    {
        return false;
    }
    uint8_t sum=0;
    for ( uint32_t i=0; i<ptr->Length; ++i )
    {
        sum+= ((const uint8_t *)ptr)[i];
    }
    return sum == 0;
}
