#include "kernel.h"

/* External function from multiboot.c */
extern void multiboot_store_info(multiboot_info_t *mbi);

/* Kernel entry point - called from boot.asm */
void kmain(multiboot_info_t *mbi) {
    /* Store multiboot info for later use */
    multiboot_store_info(mbi);

    /* Initialize serial port for debug output */
    serial_init();
    serial_puts("LeafOS Kernel Loaded\n");

    /* Initialize VGA text mode with blue background */
    vga_set_color(15, 1);  /* White text (15) on blue background (1) */
    vga_clear();

    /* Print welcome message */
    vga_puts("=====================================\n");
    vga_puts("       LeafOS Kernel v0.1\n");
    vga_puts("       x86 32-bit Protected Mode\n");
    vga_puts("=====================================\n");
    vga_puts("\n");

    /* Extract initrd information */
    uint32_t initrd_addr = get_initrd_address();
    uint32_t initrd_size = get_initrd_size();

    if (initrd_addr && initrd_size) {
        vga_printf("Initrd loaded at: 0x%x\n", initrd_addr);
        vga_printf("Initrd size: %d bytes\n", initrd_size);
        vga_puts("\nKernel: Executing initrd init process...\n");
        vga_puts("=====================================\n\n");
    } else {
        vga_puts("ERROR: No initrd module found!\n");
        halt();
    }

    /* Transfer control to initrd init process
       For now, we'll just halt since we haven't implemented the full bootstrap */
    vga_puts("\nKernel ready - passing to initrd init.\n");

    halt();
}

void delay(uint32_t ms) {
    /* Simple busy-wait delay */
    for (volatile uint32_t i = 0; i < ms * 1000; i++) {
        asm("nop");
    }
}

void halt(void) {
    vga_puts("\nSystem halted.\n");
    asm("cli");
    asm("hlt");
    while (1) {
        asm("hlt");
    }
}
