#include <stddef.h>
#include <stdint.h>

此文件不应该被编译，请使用memmove.s

void * memmove ( void * destination, const void * source, size_t num )
{
    if ( source > destination || destination >= (const void *)((const uint8_t *)source + num) )
    {
        for ( size_t i=0; i<num; ++i )
        {
            ((uint8_t*)destination)[i]=((const uint8_t*)source)[i];
        }
    }
    else if ( source < destination )
    {
        while (num!=0)
        {
            --num;
            ((uint8_t*)destination)[num]=((const uint8_t*)source)[num];
        }
    }
    return destination;
}
