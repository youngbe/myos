#include <string.h>
#include <stdint.h>

#include "RSDP.h"

// 应该有且只有一个有效的RSDP
// 参考：https://wiki.osdev.org/RSDP#Detecting_the_RSDP
// ret.status: -1 未找到， -2 找到多个，-3 未知的ACPI Version，-4 长度不符，-5 入口错误
__typeof__(find_RSDP())
find_RSDP()
{
    __typeof__(find_RSDP()) ret;
    ret.status=-1;
    const struct RSDPDescriptor20 *ptr=(const struct RSDPDescriptor20 *)( ((size_t)*(const uint16_t *)0x040E) << 4 );
    if ( ptr < (const struct RSDPDescriptor20 *)0x00080000 || ptr > (const struct RSDPDescriptor20 *)0x0009FFFF )
    {
        ret.status=-5;
        return ret;
    }
    const char temp[8]={'R', 'S', 'D', ' ', 'P', 'T', 'R', ' '};
    do
    {
        if ( strncmp( ptr->firstPart.Signature, temp, sizeof(temp) ) == 0 && valid_RSDP(ptr) )
        {
            if ( ret.status == -1 )
            {
                ret.status=0;
                ret.RSDP_ptr=ptr;
            }
            else
            {
                ret.status=-2;
                return ret;
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
            if ( ret.status == -1 )
            {
                ret.status=0;
                ret.RSDP_ptr=ptr;
            }
            else
            {
                ret.status=-2;
                return ret;
            }
        }
        ptr=(const struct RSDPDescriptor20 *)((const uint8_t *)ptr+16);
    }
    while ( ptr <= (const struct RSDPDescriptor20 *)0x000FFFFF );
    if ( ret.status == 0 )
    {
        if ( ret.RSDP_ptr->firstPart.Revision == 2 )
        {
            if ( ret.RSDP_ptr->Length != sizeof(struct RSDPDescriptor20) )
            {
                ret.status=-4;
            }
        }
        else if ( ret.RSDP_ptr->firstPart.Revision != 0 )
        {
            ret.status=-3;
        }
        else if ( ret.RSDP_ptr->Length != sizeof(struct RSDPDescriptor) )
        {
            ret.status=-4;
        }
    }
    return ret;
}

bool valid_RSDP(const struct RSDPDescriptor20* ptr)
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
