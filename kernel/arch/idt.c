#include "../include/idt.h"
#include "../include/stdint.h"

// IDT entries (256 total)
static struct idt_entry idt[256];
static struct idt_ptr idt_pointer;

// External function to load IDT (in assembly)
extern void idt_flush(uint64_t idt_ptr);

// External ISR/IRQ handlers (in assembly)
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

// I/O port functions
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void idt_set_gate(uint8_t num, void (*handler)(void), uint8_t flags) {
    uint64_t base = (uint64_t)handler;

    idt[num].offset_low = base & 0xFFFF;
    idt[num].selector = 0x08;  // Kernel code segment
    idt[num].ist = 0;
    idt[num].type_attr = flags;
    idt[num].offset_mid = (base >> 16) & 0xFFFF;
    idt[num].offset_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].zero = 0;
}

void pic_init(void) {
    // Remap PIC (Programmable Interrupt Controller)
    // Master PIC: 0x20-0x21
    // Slave PIC: 0xA0-0xA1

    // Start initialization sequence
    outb(0x20, 0x11);  // ICW1: Start initialization, expect ICW4
    outb(0xA0, 0x11);

    // Set interrupt vector offsets
    outb(0x21, 0x20);  // ICW2: Master PIC offset to 0x20 (32)
    outb(0xA1, 0x28);  // ICW2: Slave PIC offset to 0x28 (40)

    // Tell PICs about each other
    outb(0x21, 0x04);  // ICW3: Tell Master PIC there's a slave at IRQ2
    outb(0xA1, 0x02);  // ICW3: Tell Slave PIC its cascade identity

    // Set mode
    outb(0x21, 0x01);  // ICW4: 8086 mode
    outb(0xA1, 0x01);

    // Mask all interrupts initially
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

void irq_enable(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = 0x21;  // Master PIC
    } else {
        port = 0xA1;  // Slave PIC
        irq -= 8;
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

void irq_disable(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = 0x21;  // Master PIC
    } else {
        port = 0xA1;  // Slave PIC
        irq -= 8;
    }

    value = inb(port) | (1 << irq);
    outb(port, value);
}

void pic_send_eoi(uint8_t irq) {
    // Send End of Interrupt signal
    if (irq >= 8) {
        outb(0xA0, 0x20);  // Send to slave
    }
    outb(0x20, 0x20);  // Send to master
}

void idt_init(void) {
    idt_pointer.limit = (sizeof(struct idt_entry) * 256) - 1;
    idt_pointer.base = (uint64_t)&idt;

    // Clear IDT
    for (int i = 0; i < 256; i++) {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].ist = 0;
        idt[i].type_attr = 0;
        idt[i].offset_mid = 0;
        idt[i].offset_high = 0;
        idt[i].zero = 0;
    }

    // Install exception handlers (ISR 0-31)
    // Flags: 0x8E = Present, Ring 0, 64-bit interrupt gate
    idt_set_gate(0, isr0, 0x8E);
    idt_set_gate(1, isr1, 0x8E);
    idt_set_gate(2, isr2, 0x8E);
    idt_set_gate(3, isr3, 0x8E);
    idt_set_gate(4, isr4, 0x8E);
    idt_set_gate(5, isr5, 0x8E);
    idt_set_gate(6, isr6, 0x8E);
    idt_set_gate(7, isr7, 0x8E);
    idt_set_gate(8, isr8, 0x8E);
    idt_set_gate(9, isr9, 0x8E);
    idt_set_gate(10, isr10, 0x8E);
    idt_set_gate(11, isr11, 0x8E);
    idt_set_gate(12, isr12, 0x8E);
    idt_set_gate(13, isr13, 0x8E);
    idt_set_gate(14, isr14, 0x8E);
    idt_set_gate(15, isr15, 0x8E);
    idt_set_gate(16, isr16, 0x8E);
    idt_set_gate(17, isr17, 0x8E);
    idt_set_gate(18, isr18, 0x8E);
    idt_set_gate(19, isr19, 0x8E);
    idt_set_gate(20, isr20, 0x8E);
    idt_set_gate(21, isr21, 0x8E);
    idt_set_gate(22, isr22, 0x8E);
    idt_set_gate(23, isr23, 0x8E);
    idt_set_gate(24, isr24, 0x8E);
    idt_set_gate(25, isr25, 0x8E);
    idt_set_gate(26, isr26, 0x8E);
    idt_set_gate(27, isr27, 0x8E);
    idt_set_gate(28, isr28, 0x8E);
    idt_set_gate(29, isr29, 0x8E);
    idt_set_gate(30, isr30, 0x8E);
    idt_set_gate(31, isr31, 0x8E);

    // Initialize and remap PIC
    pic_init();

    // Install IRQ handlers (32-47)
    idt_set_gate(32, irq0, 0x8E);
    idt_set_gate(33, irq1, 0x8E);
    idt_set_gate(34, irq2, 0x8E);
    idt_set_gate(35, irq3, 0x8E);
    idt_set_gate(36, irq4, 0x8E);
    idt_set_gate(37, irq5, 0x8E);
    idt_set_gate(38, irq6, 0x8E);
    idt_set_gate(39, irq7, 0x8E);
    idt_set_gate(40, irq8, 0x8E);
    idt_set_gate(41, irq9, 0x8E);
    idt_set_gate(42, irq10, 0x8E);
    idt_set_gate(43, irq11, 0x8E);
    idt_set_gate(44, irq12, 0x8E);
    idt_set_gate(45, irq13, 0x8E);
    idt_set_gate(46, irq14, 0x8E);
    idt_set_gate(47, irq15, 0x8E);

    // Load IDT
    idt_flush((uint64_t)&idt_pointer);
}
