#ifndef HEAP_H
#define HEAP_H

#include "stddef.h"

// Initialize kernel heap
void heap_init(void);

// Kernel memory allocation functions
void* kmalloc(size_t size);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t new_size);

// Aligned allocation
void* kmalloc_aligned(size_t size, size_t alignment);

#endif
