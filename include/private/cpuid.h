#pragma once

#include <stddef.h>
#include <stdint.h>

static inline size_t get_cpuid()
{
    uint32_t apicid;
    __asm__ volatile
        (
         "rdmsr"
         :"=a"(apicid)
         :"c"((uint32_t)0x802)
         :
         );
    return (size_t)apicid;
}
