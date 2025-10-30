#include "../include/heap.h"
#include "../include/pmm.h"
#include "../include/stdint.h"
#include "../include/stddef.h"
#include "../include/stdbool.h"
#include "../include/vga.h"

// Heap block header
struct heap_block {
    size_t size;
    bool is_free;
    struct heap_block* next;
};

// Heap start and end
static struct heap_block* heap_start = NULL;
static void* heap_end = NULL;

#define HEAP_START 0xFFFFFFFF90000000ULL
#define HEAP_INITIAL_SIZE (1024 * 1024)  // 1MB initial heap

void heap_init(void) {
    vga_writestring("Initializing kernel heap...\n");

    // Allocate initial heap pages
    size_t pages_needed = HEAP_INITIAL_SIZE / PAGE_SIZE;
    void* heap_phys = pmm_alloc_pages(pages_needed);

    if (heap_phys == NULL) {
        vga_writestring("ERROR: Failed to allocate heap memory\n");
        return;
    }

    // For simplified implementation, use physical address directly
    // (In full implementation, would map to HEAP_START virtual address)
    heap_start = (struct heap_block*)heap_phys;
    heap_end = (void*)((uint64_t)heap_phys + HEAP_INITIAL_SIZE);

    // Initialize first block
    heap_start->size = HEAP_INITIAL_SIZE - sizeof(struct heap_block);
    heap_start->is_free = true;
    heap_start->next = NULL;

    vga_writestring("Heap initialized\n");
}

void* kmalloc(size_t size) {
    if (size == 0 || heap_start == NULL) {
        return NULL;
    }

    // Align size to 8 bytes
    size = (size + 7) & ~7;

    // Find free block (first-fit algorithm)
    struct heap_block* current = heap_start;

    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            // Found suitable block

            // Split block if remaining space is large enough
            if (current->size >= size + sizeof(struct heap_block) + 8) {
                struct heap_block* new_block = (struct heap_block*)((uint8_t*)current + sizeof(struct heap_block) + size);
                new_block->size = current->size - size - sizeof(struct heap_block);
                new_block->is_free = true;
                new_block->next = current->next;

                current->size = size;
                current->next = new_block;
            }

            current->is_free = false;

            // Return pointer after header
            return (void*)((uint8_t*)current + sizeof(struct heap_block));
        }

        current = current->next;
    }

    // No suitable block found
    return NULL;
}

void kfree(void* ptr) {
    if (ptr == NULL || heap_start == NULL) {
        return;
    }

    // Get block header
    struct heap_block* block = (struct heap_block*)((uint8_t*)ptr - sizeof(struct heap_block));

    // Validate block
    if ((void*)block < (void*)heap_start || (void*)block >= heap_end) {
        return;  // Invalid pointer
    }

    // Mark as free
    block->is_free = true;

    // Coalesce with next block if it's free
    if (block->next != NULL && block->next->is_free) {
        block->size += sizeof(struct heap_block) + block->next->size;
        block->next = block->next->next;
    }

    // Coalesce with previous block if it's free
    struct heap_block* current = heap_start;
    while (current != NULL && current->next != block) {
        current = current->next;
    }

    if (current != NULL && current->is_free) {
        current->size += sizeof(struct heap_block) + block->size;
        current->next = block->next;
    }
}

void* krealloc(void* ptr, size_t new_size) {
    if (ptr == NULL) {
        return kmalloc(new_size);
    }

    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }

    // Get current block
    struct heap_block* block = (struct heap_block*)((uint8_t*)ptr - sizeof(struct heap_block));

    // If new size fits in current block, just return same pointer
    if (block->size >= new_size) {
        return ptr;
    }

    // Allocate new block
    void* new_ptr = kmalloc(new_size);
    if (new_ptr == NULL) {
        return NULL;
    }

    // Copy old data
    uint8_t* src = (uint8_t*)ptr;
    uint8_t* dest = (uint8_t*)new_ptr;
    for (size_t i = 0; i < block->size && i < new_size; i++) {
        dest[i] = src[i];
    }

    // Free old block
    kfree(ptr);

    return new_ptr;
}

void* kmalloc_aligned(size_t size, size_t alignment) {
    // Allocate extra space for alignment
    void* ptr = kmalloc(size + alignment + sizeof(void*));
    if (ptr == NULL) {
        return NULL;
    }

    // Calculate aligned address
    uint64_t addr = (uint64_t)ptr + sizeof(void*);
    uint64_t aligned_addr = (addr + alignment - 1) & ~(alignment - 1);

    // Store original pointer before aligned address
    *((void**)(aligned_addr - sizeof(void*))) = ptr;

    return (void*)aligned_addr;
}
