#include "../include/pmm.h"
#include "../include/multiboot2.h"
#include "../include/stdint.h"
#include "../include/stddef.h"
#include "../include/stdbool.h"
#include "../include/vga.h"

// Bitmap for physical memory (1 bit per page)
static uint32_t* bitmap = NULL;
static uint64_t bitmap_size = 0;  // Size in bytes
static uint64_t total_pages = 0;
static uint64_t free_pages = 0;
static uint64_t total_memory = 0;

// Kernel end address (from linker script)
extern uint64_t _kernel_end;

// Helper: Set bit in bitmap
static inline void bitmap_set(uint64_t bit) {
    bitmap[bit / 32] |= (1 << (bit % 32));
}

// Helper: Clear bit in bitmap
static inline void bitmap_clear(uint64_t bit) {
    bitmap[bit / 32] &= ~(1 << (bit % 32));
}

// Helper: Test bit in bitmap
static inline bool bitmap_test(uint64_t bit) {
    return (bitmap[bit / 32] & (1 << (bit % 32))) != 0;
}

// Find first free page
static uint64_t find_free_page(void) {
    for (uint64_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            return i;
        }
    }
    return (uint64_t)-1;  // No free pages
}

// Find contiguous free pages
static uint64_t find_free_pages(size_t count) {
    uint64_t start = 0;
    uint64_t found = 0;

    for (uint64_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            if (found == 0) {
                start = i;
            }
            found++;
            if (found == count) {
                return start;
            }
        } else {
            found = 0;
        }
    }

    return (uint64_t)-1;  // Not enough contiguous pages
}

void pmm_init(struct multiboot_tag_mmap* mmap) {
    vga_writestring("Initializing physical memory manager...\n");

    // Find largest usable memory region to determine total memory
    uint64_t highest_address = 0;

    struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*)mmap;
    uint32_t entry_count = (mmap->size - 16) / mmap->entry_size;

    for (uint32_t i = 0; i < entry_count; i++) {
        entry = (struct multiboot_mmap_entry*)((uint8_t*)mmap + 16 + i * mmap->entry_size);

        if (entry->type == 1) {  // Available memory
            uint64_t end = entry->addr + entry->len;
            if (end > highest_address) {
                highest_address = end;
            }
        }
    }

    // Calculate total pages
    total_memory = highest_address;
    total_pages = highest_address / PAGE_SIZE;

    // Calculate bitmap size (in bytes)
    bitmap_size = (total_pages + 7) / 8;  // Round up to nearest byte

    // Place bitmap after kernel (align to page boundary)
    uint64_t kernel_end = (uint64_t)&_kernel_end;
    kernel_end = (kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);  // Align to page
    bitmap = (uint32_t*)kernel_end;

    // Clear bitmap (mark all pages as used initially)
    for (uint64_t i = 0; i < bitmap_size / 4; i++) {
        bitmap[i] = 0xFFFFFFFF;
    }

    free_pages = 0;

    // Mark available regions as free
    for (uint32_t i = 0; i < entry_count; i++) {
        entry = (struct multiboot_mmap_entry*)((uint8_t*)mmap + 16 + i * mmap->entry_size);

        if (entry->type == 1) {  // Available memory
            uint64_t start_page = entry->addr / PAGE_SIZE;
            uint64_t end_page = (entry->addr + entry->len) / PAGE_SIZE;

            for (uint64_t page = start_page; page < end_page; page++) {
                if (page < total_pages) {
                    bitmap_clear(page);
                    free_pages++;
                }
            }
        }
    }

    // Reserve kernel memory and bitmap
    uint64_t kernel_start = 0x100000;  // 1MB (where kernel is loaded)
    uint64_t bitmap_end = (uint64_t)bitmap + bitmap_size;
    uint64_t reserved_end = (bitmap_end + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint64_t page = kernel_start / PAGE_SIZE; page <= reserved_end; page++) {
        if (page < total_pages && !bitmap_test(page)) {
            bitmap_set(page);
            free_pages--;
        }
    }

    // Reserve low memory (first 1MB)
    for (uint64_t page = 0; page < 256; page++) {  // 256 pages = 1MB
        if (!bitmap_test(page)) {
            bitmap_set(page);
            free_pages--;
        }
    }

    vga_writestring("PMM initialized: ");
    // Print memory stats (simple output)
    vga_writestring("Total memory detected\n");
}

void* pmm_alloc_page(void) {
    uint64_t page = find_free_page();
    if (page == (uint64_t)-1) {
        return NULL;  // Out of memory
    }

    bitmap_set(page);
    free_pages--;

    return (void*)(page * PAGE_SIZE);
}

void pmm_free_page(void* page) {
    if (page == NULL) {
        return;
    }

    uint64_t page_num = (uint64_t)page / PAGE_SIZE;
    if (page_num >= total_pages) {
        return;  // Invalid page
    }

    if (bitmap_test(page_num)) {
        bitmap_clear(page_num);
        free_pages++;
    }
}

void* pmm_alloc_pages(size_t count) {
    if (count == 0) {
        return NULL;
    }

    uint64_t start_page = find_free_pages(count);
    if (start_page == (uint64_t)-1) {
        return NULL;  // Not enough contiguous memory
    }

    for (size_t i = 0; i < count; i++) {
        bitmap_set(start_page + i);
        free_pages--;
    }

    return (void*)(start_page * PAGE_SIZE);
}

void pmm_free_pages(void* page, size_t count) {
    if (page == NULL || count == 0) {
        return;
    }

    uint64_t page_num = (uint64_t)page / PAGE_SIZE;

    for (size_t i = 0; i < count; i++) {
        uint64_t p = page_num + i;
        if (p < total_pages && bitmap_test(p)) {
            bitmap_clear(p);
            free_pages++;
        }
    }
}

uint64_t pmm_get_total_memory(void) {
    return total_memory;
}

uint64_t pmm_get_free_memory(void) {
    return free_pages * PAGE_SIZE;
}
