#include "kernel.h"

/* Serial port COM1 is at 0x3F8 */
#define SERIAL_PORT 0x3F8

/* Register offsets */
#define SERIAL_THR 0  /* Transmitter Holding Register */
#define SERIAL_RBR 0  /* Receiver Buffer Register */
#define SERIAL_IER 1  /* Interrupt Enable Register */
#define SERIAL_FCR 2  /* FIFO Control Register */
#define SERIAL_LCR 3  /* Line Control Register */
#define SERIAL_MCR 4  /* Modem Control Register */
#define SERIAL_LSR 5  /* Line Status Register */
#define SERIAL_MSR 6  /* Modem Status Register */
#define SERIAL_DLL 0  /* Divisor Latch Low (when LCR bit 7 = 1) */
#define SERIAL_DLH 1  /* Divisor Latch High (when LCR bit 7 = 1) */

/* Line Status Register bits */
#define SERIAL_LSR_EMPTY 0x20  /* Transmitter Holding Register is empty */

static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    asm("inb %1, %0" : "=a" (val) : "d" (port));
    return val;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm("outb %0, %1" : : "a" (val), "d" (port));
}

void serial_init(void) {
    /* Disable interrupts */
    outb(SERIAL_PORT + SERIAL_IER, 0x00);

    /* Enable DLAB to set baud rate */
    outb(SERIAL_PORT + SERIAL_LCR, 0x80);

    /* Set baud rate to 115200
       Divisor = 115200 / 115200 = 1
       DLL = 1, DLH = 0 */
    outb(SERIAL_PORT + SERIAL_DLL, 0x01);
    outb(SERIAL_PORT + SERIAL_DLH, 0x00);

    /* 8 bits, 1 stop bit, no parity - set LCR to 0x03
       Disable DLAB bit */
    outb(SERIAL_PORT + SERIAL_LCR, 0x03);

    /* Enable FIFO and clear queues */
    outb(SERIAL_PORT + SERIAL_FCR, 0xC7);

    /* Set RTS/DTR */
    outb(SERIAL_PORT + SERIAL_MCR, 0x0B);
}

static void serial_wait_transmit(void) {
    /* Wait until transmitter is empty */
    while (!(inb(SERIAL_PORT + SERIAL_LSR) & SERIAL_LSR_EMPTY)) {
        /* Busy wait */
    }
}

void serial_putchar(char c) {
    serial_wait_transmit();
    outb(SERIAL_PORT + SERIAL_THR, (uint8_t)c);
}

void serial_puts(const char *s) {
    while (*s) {
        serial_putchar(*s);
        s++;
    }
}
