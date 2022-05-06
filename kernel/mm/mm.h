#include <stddef.h>

void *kmalloc(size_t);
void *kmalloc_p2align(size_t, size_t);
void kfree(void *);
