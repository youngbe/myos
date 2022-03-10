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
static inline const void *find_RSDP();

static inline bool vaild_RSDP(const struct RSDPDescriptor20* ptr);

ssize_t init_x2apic()
{
    const struct RSDPDescriptor20* ptr=find_RSDP();
    if (ptr == NULL)
    {
        return -1;
    }
    if ( ptr->firstPart.Revision != 2 )
    {
        return -1;
    }
    if ( ptr->Length != sizeof(struct RSDPDescriptor20) )
    {
        return -1;
    }
    return 0;
}

// 应该有且只有一个有效的RSDP
// 参考：https://wiki.osdev.org/RSDP#Detecting_the_RSDP
inline const void *find_RSDP()
{
    const void *result=NULL;
    const void *ptr=(const void *)( (*(const uint32_t *)0x040E) << 4 );
    if ( ptr < (const void *)0x00080000 || ptr > (const void *)0x0009FFFF )
    {
        return NULL;
    }
    const char temp[8]={'R', 'S', 'D', ' ', 'P', 'T', 'R', ' '};
    while ( ptr <= (const void *)0x0009FFFF )
    {
        if ( strncmp( ptr, temp, 8 ) == 0 && vaild_RSDP(ptr) )
        {
            if ( result == NULL )
            {
                result=ptr;
            }
            else
            {
                return NULL;
            }
            //return ptr;
        }
        ptr=(const uint8_t *)ptr+16;
    }
    ptr=(const void *)0x000E0000;
    do
    {
        if ( strncmp( ptr, temp, 8 ) == 0 && vaild_RSDP(ptr) )
        {
            if ( result == NULL )
            {
                result=ptr;
            }
            else
            {
                return NULL;
            }
            //return ptr;
        }
        ptr=(const uint8_t *)ptr+16;
    }
    while ( ptr <= (void *)0x000FFFFF );
    return result;
}

inline bool vaild_RSDP(const struct RSDPDescriptor20* ptr)
{
    {
        uint8_t temp_sum=0;
        for ( size_t i=0; i<sizeof(struct RSDPDescriptor); ++i )
        {
            temp_sum+=((const uint8_t *)ptr)[i];
        }
        if (temp_sum != 0)
        {
            return false;
        }
    }
    if ( ptr->firstPart.Revision >= 2 )
    {
        uint8_t temp_sum=0;
        for ( size_t i=sizeof(struct RSDPDescriptor); i<sizeof(struct RSDPDescriptor20); ++i )
        {
            temp_sum+=((const uint8_t *)ptr)[i];
        }
        if (temp_sum != 0)
        {
            return false;
        }
    }
    return true;
}
