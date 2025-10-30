#ifndef VMM_H
#define VMM_H

#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"

// Page flags
#define PAGE_PRESENT    (1 << 0)
#define PAGE_WRITE      (1 << 1)
#define PAGE_USER       (1 << 2)
#define PAGE_WRITETHROUGH (1 << 3)
#define PAGE_CACHEDISABLE (1 << 4)
#define PAGE_ACCESSED   (1 << 5)
#define PAGE_DIRTY      (1 << 6)
#define PAGE_HUGE       (1 << 7)

// Initialize virtual memory manager
void vmm_init(void);

// Map virtual address to physical address
void* vmm_map_page(void* virt, void* phys, uint32_t flags);

// Unmap virtual address
void vmm_unmap_page(void* virt);

// Create new address space (returns PML4 physical address)
void* vmm_create_address_space(void);

// Switch to address space
void vmm_switch_address_space(void* pml4);

// Page fault handler
void page_fault_handler(uint64_t error_code, uint64_t address);

#endif
