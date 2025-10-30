#include "include/stdint.h"
#include "include/stddef.h"
#include "include/stdbool.h"
#include "include/vga.h"
#include "include/multiboot2.h"

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
                // We'll process this in Phase 2 for memory management
                break;
            }
        }
    }

    vga_writestring("\n");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_writestring("LeafOS kernel initialized successfully!\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_writestring("\n");
    vga_writestring("Phase 1 complete: Boot and basic output working\n");
    vga_writestring("Remaining phases will be implemented progressively:\n");
    vga_writestring("- Phase 2: Interrupts and Memory\n");
    vga_writestring("- Phase 3: Input and Timing\n");
    vga_writestring("- Phase 4: Process Management\n");
    vga_writestring("- Phase 5: Disk and File System\n");
    vga_writestring("- Phase 6: Standard Library\n");
    vga_writestring("- Phase 7: Userspace Applications\n");
    vga_writestring("- Phase 8: Installation System\n");
    vga_writestring("\n");

halt:
    // Hang forever
    while (1) {
        __asm__ volatile ("hlt");
    }
}
