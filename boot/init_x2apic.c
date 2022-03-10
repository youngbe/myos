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

struct MADT
{
    struct ACPISDTHeader h;
    uint32_t local_apic_address;
    uint32_t flag;
} __attribute__ ((packed));

// 找到有且仅有的一个有效RSDP
static inline const struct RSDPDescriptor20 *find_RSDP();

static inline bool valid_RSDP(const struct RSDPDescriptor20* ptr);

static inline bool valid_ACPISDT(const struct ACPISDTHeader* ptr);
static inline size_t get_madt_list(const struct MADT** madt_ptr_list, const struct XSDT* xsdt_ptr);

ssize_t init_x2apic()
{
    const struct XSDT* xsdt_ptr;
    {
        const struct RSDPDescriptor20* ptr=find_RSDP();
        if (ptr == NULL)
        {
            return -1;
        }
        if ( ptr->firstPart.Revision != 2 )
        {
            return -2;
        }
        if ( ptr->Length != sizeof(struct RSDPDescriptor20) )
        {
            return -3;
        }
        xsdt_ptr=(const struct XSDT*)ptr->XsdtAddress;
    }
    if ( !valid_ACPISDT((const struct ACPISDTHeader*)xsdt_ptr) )
    {
        return -4;
    }
    {
        const char temp[]={'X', 'S', 'D', 'T'};
        if ( strncmp(xsdt_ptr->h.Signature, temp, sizeof(temp)) != 0 )
        {
            return -5;
        }
    }
    const struct MADT *madt_ptr_list[(xsdt_ptr->h.Length-sizeof(struct ACPISDTHeader)) /sizeof(uint64_t)];
    size_t madt_nums=get_madt_list(madt_ptr_list, xsdt_ptr);
    if ( madt_nums == 0 )
    {
        return -6;
    }
    return madt_nums;
    return 0;
}

inline size_t get_madt_list(const struct MADT** madt_ptr_list, const struct XSDT* xsdt_ptr)
{
    size_t num=0;
    const char temp[4]={'A', 'P', 'I', 'C'};
    for ( size_t i=0, sum=(xsdt_ptr->h.Length-sizeof(struct ACPISDTHeader)) /sizeof(uint64_t); i<sum ; ++i )
    {
        if ( valid_ACPISDT( (const struct ACPISDTHeader *)xsdt_ptr->PointerToOtherSDT[i]) )
        {
            if ( strncmp( ((const struct ACPISDTHeader *)xsdt_ptr->PointerToOtherSDT[i])->Signature, temp, sizeof(temp) ) == 0 )
            {
                madt_ptr_list[num]=(const struct MADT *)xsdt_ptr->PointerToOtherSDT[i];
                ++num;
            }
        }
    }
    return num;
}

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

// 应该有且只有一个有效的RSDP
// 参考：https://wiki.osdev.org/RSDP#Detecting_the_RSDP
inline const struct RSDPDescriptor20 *find_RSDP()
{
    const struct RSDPDescriptor20 *result=NULL;
    const struct RSDPDescriptor20 *ptr=(const struct RSDPDescriptor20 *)( ((size_t)*(const uint16_t *)0x040E) << 4 );
    if ( ptr < (const struct RSDPDescriptor20 *)0x00080000 || ptr > (const struct RSDPDescriptor20 *)0x0009FFFF )
    {
        return NULL;
    }
    const char temp[8]={'R', 'S', 'D', ' ', 'P', 'T', 'R', ' '};
    do
    {
        if ( strncmp( ptr->firstPart.Signature, temp, sizeof(temp) ) == 0 && valid_RSDP(ptr) )
        {
            if ( result == NULL )
            {
                result=ptr;
            }
            else
            {
                return NULL;
            }
        }
        ptr=(const struct RSDPDescriptor20 *)((const uint8_t *)ptr+16);
    }
    while ( ptr <= (const struct RSDPDescriptor20 *)0x0009FFFF );
    ptr=(const struct RSDPDescriptor20 *)0x000E0000;
    do
    {
        if ( strncmp( ptr->firstPart.Signature, temp, 8 ) == 0 && valid_RSDP(ptr) )
        {
            if ( result == NULL )
            {
                result=ptr;
            }
            else
            {
                return NULL;
            }
        }
        ptr=(const struct RSDPDescriptor20 *)((const uint8_t *)ptr+16);
    }
    while ( ptr <= (const struct RSDPDescriptor20 *)0x000FFFFF );
    return result;
}

inline bool valid_RSDP(const struct RSDPDescriptor20* ptr)
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
