#include <string.h>
#include <stdint.h>

void * memset ( void * ptr, int value, size_t num )
{
    while ( num != 0 )
    {
        --num;
        ((uint8_t *)ptr)[num]=(uint8_t)value;
    }
    return ptr;
}

void * memcpy ( void * destination, const void * source, size_t num )
{
    for ( size_t i=0; i<num; ++i )
    {
        ((uint8_t*)destination)[i]=((const uint8_t*)source)[i];
    }
    return destination;
}
