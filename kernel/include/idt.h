#ifndef IDT_H
#define IDT_H

#include "stdint.h"

// IDT entry structure
struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

// IDT pointer structure
struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

// Initialize IDT
void idt_init(void);

// Set IDT gate
void idt_set_gate(uint8_t num, void (*handler)(void), uint8_t flags);

// PIC functions
void pic_init(void);
void irq_enable(uint8_t irq);
void irq_disable(uint8_t irq);
void pic_send_eoi(uint8_t irq);

#endif
