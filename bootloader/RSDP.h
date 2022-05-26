#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct __attribute__((packed)) RSDPDescriptor
{
    char Signature[8];
    uint8_t Checksum;
    char OEMID[6];
    uint8_t Revision;
    uint32_t RsdtAddress;
};

struct __attribute__((packed)) RSDPDescriptor20
{
    struct RSDPDescriptor firstPart;
    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    uint8_t reserved[3];
};

// 找到有且仅有的一个有效RSDP
struct
{
    ssize_t status;
    const struct RSDPDescriptor20 * RSDP_ptr;
}
find_RSDP();

bool valid_RSDP(const struct RSDPDescriptor20* ptr);
