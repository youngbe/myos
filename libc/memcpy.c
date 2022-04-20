#include <string.h>
#include <stdint.h>

这个文件不应该被编译，请使用memcpy.s

void * memcpy ( void * destination, const void * source, size_t num )
{
    for ( size_t i=0; i<num; ++i )
    {
        ((uint8_t*)destination)[i]=((const uint8_t*)source)[i];
    }
    return destination;
}
