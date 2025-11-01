#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

/* ==================== Types ==================== */

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint32_t vbe_mode;
    uint32_t vbe_interface_seg;
    uint32_t vbe_interface_off;
    uint32_t vbe_interface_len;
} multiboot_info_t;

typedef struct {
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t cmdline;
    uint32_t reserved;
} multiboot_module_t;

typedef struct {
    uint32_t size;
    uint32_t addr;
    uint32_t len;
    uint32_t type;
} multiboot_memory_map_t;

/* ==================== Kernel Entry ==================== */

extern void kmain(multiboot_info_t *mbi);

/* ==================== VGA Output ==================== */

void vga_clear(void);
void vga_putchar(char c);
void vga_puts(const char *s);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_printf(const char *format, ...);

/* ==================== Serial Debug Output ==================== */

void serial_init(void);
void serial_putchar(char c);
void serial_puts(const char *s);

/* ==================== Multiboot & Module Handling ==================== */

multiboot_info_t *get_multiboot_info(void);
uint32_t get_initrd_address(void);
uint32_t get_initrd_size(void);

/* ==================== Utilities ==================== */

void delay(uint32_t ms);
void halt(void);

#endif // KERNEL_H
