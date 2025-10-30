#include "../include/vmm.h"
#include "../include/pmm.h"
#include "../include/stdint.h"
#include "../include/stddef.h"
#include "../include/stdbool.h"
#include "../include/vga.h"

// Current kernel page directory (PML4)
static void* kernel_pml4 = NULL;

// Helper: Get CR3 register
static inline uint64_t get_cr3(void) {
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

// Helper: Set CR3 register
static inline void set_cr3(uint64_t cr3) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(cr3) : "memory");
}

void vmm_init(void) {
    vga_writestring("Initializing virtual memory manager...\n");

    // Get current PML4 (set up by boot.asm)
    kernel_pml4 = (void*)get_cr3();

    vga_writestring("VMM initialized\n");
}

void* vmm_map_page(void* virt, void* phys, uint32_t flags) {
    // Simplified mapping - full implementation would manipulate page tables
    // For now, we rely on the identity mapping set up in boot.asm

    // In a full implementation:
    // 1. Extract PML4, PDPT, PD, PT indices from virtual address
    // 2. Ensure page table structure exists (allocate if needed)
    // 3. Set page table entry with physical address and flags
    // 4. Invalidate TLB for the virtual address

    return virt;
}

void vmm_unmap_page(void* virt) {
    // Simplified unmap - would clear page table entry in full implementation
    (void)virt;
}

void* vmm_create_address_space(void) {
    // Allocate new PML4
    void* new_pml4 = pmm_alloc_page();
    if (new_pml4 == NULL) {
        return NULL;
    }

    // Clear the new PML4
    uint64_t* pml4_entries = (uint64_t*)new_pml4;
    for (int i = 0; i < 512; i++) {
        pml4_entries[i] = 0;
    }

    // Copy kernel mappings from current PML4
    // (Upper half should be shared across all address spaces)
    uint64_t* kernel_pml4_entries = (uint64_t*)kernel_pml4;
    for (int i = 256; i < 512; i++) {  // Copy upper half
        pml4_entries[i] = kernel_pml4_entries[i];
    }

    return new_pml4;
}

void vmm_switch_address_space(void* pml4) {
    if (pml4 != NULL) {
        set_cr3((uint64_t)pml4);
    }
}

void page_fault_handler(uint64_t error_code, uint64_t address) {
    // Page fault occurred
    vga_set_color(VGA_COLOR_RED, VGA_COLOR_BLACK);
    vga_writestring("\n\nPAGE FAULT at address: ");

    // Print address (simplified)
    vga_writestring("\nError code: ");

    vga_writestring("\nSystem halted.\n");

    // Halt
    while (1) {
        __asm__ volatile("cli; hlt");
    }
}
