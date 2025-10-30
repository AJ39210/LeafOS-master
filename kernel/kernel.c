#include "include/stdint.h"
#include "include/stddef.h"
#include "include/stdbool.h"
#include "include/vga.h"
#include "include/multiboot2.h"
#include "include/gdt.h"
#include "include/idt.h"
#include "include/pmm.h"
#include "include/vmm.h"
#include "include/heap.h"
#include "include/interrupts.h"

void kernel_main(uint32_t multiboot_magic, void* multiboot_info_addr) {
    // Initialize VGA text mode
    vga_init();
    vga_clear();

    // Print boot message
    vga_writestring("LeafOS Booting...\n");

    // Verify Multiboot2 magic number
    if (multiboot_magic != MULTIBOOT2_MAGIC) {
        vga_set_color(VGA_COLOR_RED, VGA_COLOR_BLACK);
        vga_writestring("ERROR: Invalid Multiboot2 magic number!\n");
        goto halt;
    }

    vga_writestring("Multiboot2 magic verified\n");

    // Parse Multiboot2 information structure
    struct multiboot_tag *tag;
    struct multiboot_tag_mmap *mmap_tag = NULL;
    uint32_t total_size = *(uint32_t*)multiboot_info_addr;

    vga_writestring("Parsing Multiboot2 info...\n");

    // Iterate through Multiboot2 tags
    for (tag = (struct multiboot_tag*)(multiboot_info_addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag*)((uint8_t*)tag + ((tag->size + 7) & ~7))) {

        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE: {
                struct multiboot_tag_string* cmd_tag = (struct multiboot_tag_string*)tag;
                vga_writestring("Boot command line: ");
                vga_writestring(cmd_tag->string);
                vga_writestring("\n");
                break;
            }
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
                struct multiboot_tag_string* loader_tag = (struct multiboot_tag_string*)tag;
                vga_writestring("Boot loader: ");
                vga_writestring(loader_tag->string);
                vga_writestring("\n");
                break;
            }
            case MULTIBOOT_TAG_TYPE_MMAP: {
                vga_writestring("Memory map found\n");
                mmap_tag = (struct multiboot_tag_mmap*)tag;
                break;
            }
        }
    }

    vga_writestring("\n=== Phase 2: Interrupts and Memory ===\n");

    // Initialize GDT
    vga_writestring("Initializing GDT...\n");
    gdt_init();

    // Initialize IDT
    vga_writestring("Initializing IDT...\n");
    idt_init();

    // Initialize Physical Memory Manager
    if (mmap_tag != NULL) {
        pmm_init(mmap_tag);
    } else {
        vga_set_color(VGA_COLOR_RED, VGA_COLOR_BLACK);
        vga_writestring("ERROR: No memory map provided by bootloader!\n");
        goto halt;
    }

    // Initialize Virtual Memory Manager
    vmm_init();

    // Initialize Kernel Heap
    heap_init();

    // Enable interrupts
    vga_writestring("Enabling interrupts...\n");
    sti();

    vga_writestring("\n");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_writestring("LeafOS kernel initialized successfully!\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_writestring("\n");
    vga_writestring("Phase 1-2 complete!\n");
    vga_writestring("- [X] Phase 1: Boot and basic output\n");
    vga_writestring("- [X] Phase 2: Interrupts and Memory\n");
    vga_writestring("\nRemaining phases need implementation:\n");
    vga_writestring("- [ ] Phase 3: Input and Timing (keyboard, ACPI, timer)\n");
    vga_writestring("- [ ] Phase 4: Process Management (scheduler, syscalls)\n");
    vga_writestring("- [ ] Phase 5: Disk and File System (ATA, FAT32)\n");
    vga_writestring("- [ ] Phase 6: Standard Library (printf, malloc, strings)\n");
    vga_writestring("- [ ] Phase 7: Userspace Apps (shell, calculator, notepad)\n");
    vga_writestring("- [ ] Phase 8: Installation System\n");
    vga_writestring("\n");

halt:
    // Hang forever
    while (1) {
        __asm__ volatile ("hlt");
    }
}
