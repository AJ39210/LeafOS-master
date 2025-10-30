#include "../include/interrupts.h"
#include "../include/idt.h"
#include "../include/vga.h"
#include "../include/stdint.h"

// Exception names
static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved"
};

// Helper function to print hex number
static void print_hex(uint64_t value) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[17];
    buffer[16] = '\0';

    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }

    vga_writestring("0x");
    vga_writestring(buffer);
}

// Exception handler
void exception_handler(struct interrupt_frame* frame) {
    // Set red color for errors
    vga_set_color(VGA_COLOR_RED, VGA_COLOR_BLACK);

    vga_writestring("\n\n=== KERNEL PANIC ===\n");
    vga_writestring("Exception: ");

    if (frame->int_no < 32) {
        vga_writestring(exception_messages[frame->int_no]);
    } else {
        vga_writestring("Unknown Exception");
    }

    vga_writestring("\nError Code: ");
    print_hex(frame->err_code);

    vga_writestring("\n\nRegister Dump:\n");
    vga_writestring("RAX: "); print_hex(frame->rax); vga_writestring("\n");
    vga_writestring("RBX: "); print_hex(frame->rbx); vga_writestring("\n");
    vga_writestring("RCX: "); print_hex(frame->rcx); vga_writestring("\n");
    vga_writestring("RDX: "); print_hex(frame->rdx); vga_writestring("\n");
    vga_writestring("RSI: "); print_hex(frame->rsi); vga_writestring("\n");
    vga_writestring("RDI: "); print_hex(frame->rdi); vga_writestring("\n");
    vga_writestring("RBP: "); print_hex(frame->rbp); vga_writestring("\n");
    vga_writestring("RSP: "); print_hex(frame->rsp); vga_writestring("\n");
    vga_writestring("RIP: "); print_hex(frame->rip); vga_writestring("\n");

    // Special handling for page fault
    if (frame->int_no == 14) {
        uint64_t faulting_address;
        __asm__ volatile("mov %%cr2, %0" : "=r"(faulting_address));
        vga_writestring("Faulting Address: ");
        print_hex(faulting_address);
        vga_writestring("\n");
    }

    vga_writestring("\nSystem Halted.\n");

    // Halt the system
    cli();
    while (1) {
        hlt();
    }
}

// IRQ handler
void irq_handler(struct interrupt_frame* frame) {
    // Get IRQ number (subtract 32 from interrupt number)
    uint8_t irq = frame->int_no - 32;

    // For now, just acknowledge the interrupt
    // Specific handlers will be added in Phase 3 and later

    // Send End of Interrupt (EOI) signal
    pic_send_eoi(irq);
}
