#pragma once

#ifdef __cplusplus
extern "C" {
#endif
static inline void qsort( void* base, size_t num, size_t width, int(*compare)(const void*,const void*) );
#ifdef __cplusplus
}
#endif
