#include "../include/gdt.h"
#include "../include/stdint.h"

// GDT entries
static struct gdt_entry gdt[5];
static struct gdt_ptr gdt_pointer;

// External function to load GDT (in assembly)
extern void gdt_flush(uint64_t gdt_ptr);

// Set a GDT entry
static void gdt_set_gate(int num, uint64_t base, uint64_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access = access;
}

void gdt_init(void) {
    gdt_pointer.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gdt_pointer.base = (uint64_t)&gdt;

    // Null descriptor (required)
    gdt_set_gate(0, 0, 0, 0, 0);

    // Kernel code segment (64-bit)
    // Base = 0, Limit = 0xFFFFF (4GB with granularity)
    // Access = 0x9A (present, ring 0, executable, readable)
    // Granularity = 0xA0 (64-bit, 4KB granularity)
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xA0);

    // Kernel data segment (64-bit)
    // Access = 0x92 (present, ring 0, data, writable)
    // Granularity = 0xC0 (4KB granularity)
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xC0);

    // User code segment (64-bit)
    // Access = 0xFA (present, ring 3, executable, readable)
    // Granularity = 0xA0 (64-bit, 4KB granularity)
    gdt_set_gate(3, 0, 0xFFFFF, 0xFA, 0xA0);

    // User data segment (64-bit)
    // Access = 0xF2 (present, ring 3, data, writable)
    // Granularity = 0xC0 (4KB granularity)
    gdt_set_gate(4, 0, 0xFFFFF, 0xF2, 0xC0);

    // Load the GDT
    gdt_flush((uint64_t)&gdt_pointer);
}
