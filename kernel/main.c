#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int main()
{
    *(char *)(0xb8000)='H';
    *(char *)(0xb8002)='e';
    *(char *)(0xb8004)='l';
    *(char *)(0xb8006)='l';
    *(char *)(0xb8008)='o';
    while (true)
    {
    }
}
