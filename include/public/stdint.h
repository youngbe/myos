#pragma once
#if defined __x86_64__ && !defined __ILP32__
typedef unsigned long int uint64_t;
typedef signed long int int64_t;
#else
typedef unsigned long long int uint64_t;
typedef signed long long int int64_t;
#endif
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned short int uint16_t;
typedef signed short int int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
