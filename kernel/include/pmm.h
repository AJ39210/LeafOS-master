#ifndef PMM_H
#define PMM_H

#include "stdint.h"
#include "stddef.h"
#include "multiboot2.h"

// Page size
#define PAGE_SIZE 4096

// Initialize physical memory manager
void pmm_init(struct multiboot_tag_mmap* mmap);

// Allocate single page (returns physical address)
void* pmm_alloc_page(void);

// Free single page
void pmm_free_page(void* page);

// Allocate multiple contiguous pages
void* pmm_alloc_pages(size_t count);

// Free multiple pages
void pmm_free_pages(void* page, size_t count);

// Get total and free memory
uint64_t pmm_get_total_memory(void);
uint64_t pmm_get_free_memory(void);

#endif
