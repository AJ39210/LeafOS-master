#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "stdint.h"

// Interrupt frame structure (pushed by CPU and our handlers)
struct interrupt_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));

// Exception handlers (0-31)
void exception_handler(struct interrupt_frame* frame);

// IRQ handlers (32-47)
void irq_handler(struct interrupt_frame* frame);

// Enable/disable interrupts
static inline void cli(void) { __asm__ volatile("cli"); }
static inline void sti(void) { __asm__ volatile("sti"); }
static inline void hlt(void) { __asm__ volatile("hlt"); }

#endif
