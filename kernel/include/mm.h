#pragma once
#include <stddef.h>

void *kmalloc(size_t size);
void kfree(void *base);
void *kmalloc_p2align(size_t size, size_t p2align);
