#include <string.h>
#include <stdint.h>

这个文件不应该被编译，请使用memset.s

void * memset ( void * ptr, int value, size_t num )
{
    while ( num != 0 )
    {
        --num;
        ((uint8_t *)ptr)[num]=(uint8_t)value;
    }
    return ptr;
}
