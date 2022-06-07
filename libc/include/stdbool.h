#pragma once
#ifdef __cplusplus
typedef bool _Bool;
#else
typedef _Bool bool;
#define true ((_Bool)+1u)
#define false ((_Bool)+0u)
#endif
