#pragma once
#include "libc.h"

struct RSDPDescriptor
{
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
} __attribute__ ((packed));

struct RSDPDescriptor20
{
    struct RSDPDescriptor firstPart;
    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
} __attribute__ ((packed));

// 找到有且仅有的一个有效RSDP
struct
{
    ssize_t status;
    const struct RSDPDescriptor20 * RSDP_ptr;
}
find_RSDP();

bool valid_RSDP(const struct RSDPDescriptor20* ptr);
