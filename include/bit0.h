#pragma once

// Only Support int type like: int, long int, uint64_t, size_t
#define GET_BITS_LOW(var, bit_nums) ( (var) & ( ( ((__general_typeof__(var))1) << (bit_nums) )-((__general_typeof__(var))1) ) )

#define REMOVE_BITS_LOW(var, bit_nums) ( (var) & ( ((__general_typeof__(var))-1) << (bit_nums) ) )
