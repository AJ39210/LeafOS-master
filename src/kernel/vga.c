#include "kernel.h"

#define VGA_MEMORY ((uint8_t *)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;
static uint8_t current_color = 0x07;  // White on black by default

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
    return (bg << 4) | fg;
}

static inline uint16_t vga_entry(unsigned char ch, uint8_t color) {
    return ((uint16_t)color << 8) | ch;
}

void vga_clear(void) {
    uint16_t blank = vga_entry(' ', current_color);
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            ((uint16_t *)VGA_MEMORY)[y * VGA_WIDTH + x] = blank;
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

static void vga_scroll_up(void) {
    uint16_t blank = vga_entry(' ', current_color);
    // Shift all lines up by one
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            ((uint16_t *)VGA_MEMORY)[y * VGA_WIDTH + x] =
                ((uint16_t *)VGA_MEMORY)[(y + 1) * VGA_WIDTH + x];
        }
    }
    // Clear last line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        ((uint16_t *)VGA_MEMORY)[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blank;
    }
    cursor_y = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            vga_scroll_up();
        }
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
            if (cursor_y >= VGA_HEIGHT) {
                vga_scroll_up();
            }
        }
    } else {
        size_t index = cursor_y * VGA_WIDTH + cursor_x;
        ((uint16_t *)VGA_MEMORY)[index] = vga_entry(c, current_color);
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
            if (cursor_y >= VGA_HEIGHT) {
                vga_scroll_up();
            }
        }
    }
}

void vga_puts(const char *s) {
    while (*s) {
        vga_putchar(*s);
        s++;
    }
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    current_color = vga_entry_color(fg, bg);
}

void vga_printf(const char *format, ...) {
    // Simple printf implementation - handles basic formats
    __builtin_va_list args;
    __builtin_va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {
                    int val = __builtin_va_arg(args, int);
                    if (val < 0) {
                        vga_putchar('-');
                        val = -val;
                    }
                    if (val == 0) {
                        vga_putchar('0');
                    } else {
                        int divisor = 1000000000;
                        int started = 0;
                        while (divisor > 0) {
                            int digit = val / divisor;
                            if (digit || started) {
                                vga_putchar('0' + digit);
                                started = 1;
                            }
                            val %= divisor;
                            divisor /= 10;
                        }
                    }
                    break;
                }
                case 'x': {
                    uint32_t val = __builtin_va_arg(args, uint32_t);
                    vga_puts("0x");
                    for (int i = 7; i >= 0; i--) {
                        int digit = (val >> (i * 4)) & 0xF;
                        vga_putchar(digit < 10 ? '0' + digit : 'a' + digit - 10);
                    }
                    break;
                }
                case 's': {
                    const char *str = __builtin_va_arg(args, const char *);
                    vga_puts(str);
                    break;
                }
                case 'c': {
                    char ch = __builtin_va_arg(args, int);
                    vga_putchar(ch);
                    break;
                }
                case '%':
                    vga_putchar('%');
                    break;
                default:
                    vga_putchar('%');
                    vga_putchar(*format);
                    break;
            }
        } else {
            vga_putchar(*format);
        }
        format++;
    }

    __builtin_va_end(args);
}
