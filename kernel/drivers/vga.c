#include "../include/vga.h"
#include "../include/stdint.h"
#include "../include/stddef.h"

// VGA text mode buffer
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

// Current cursor position
static size_t vga_row = 0;
static size_t vga_column = 0;

// Current color
static uint8_t vga_color = 0x0F; // White on black

// Helper function to create VGA entry
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | ((uint16_t)color << 8);
}

// Helper function to create color byte
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | (bg << 4);
}

void vga_init(void) {
    vga_row = 0;
    vga_column = 0;
    vga_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_color = vga_entry_color((enum vga_color)fg, (enum vga_color)bg);
}

void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', vga_color);
        }
    }
    vga_row = 0;
    vga_column = 0;
}

void vga_scroll(void) {
    // Move all rows up by one
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[y * VGA_WIDTH + x] = VGA_MEMORY[(y + 1) * VGA_WIDTH + x];
        }
    }

    // Clear the last row
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_color);
    }

    // Move cursor up one row
    if (vga_row > 0) {
        vga_row--;
    }
}

void vga_putchar(char c) {
    // Handle special characters
    if (c == '\n') {
        vga_column = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_column = 0;
    } else if (c == '\t') {
        vga_column = (vga_column + 8) & ~7; // Align to next 8-character boundary
    } else if (c == '\b') {
        if (vga_column > 0) {
            vga_column--;
            const size_t index = vga_row * VGA_WIDTH + vga_column;
            VGA_MEMORY[index] = vga_entry(' ', vga_color);
        }
        return; // Don't check for scroll after backspace
    } else {
        // Normal character
        const size_t index = vga_row * VGA_WIDTH + vga_column;
        VGA_MEMORY[index] = vga_entry(c, vga_color);
        vga_column++;
    }

    // Handle line wrap
    if (vga_column >= VGA_WIDTH) {
        vga_column = 0;
        vga_row++;
    }

    // Handle scroll
    if (vga_row >= VGA_HEIGHT) {
        vga_scroll();
    }
}

void vga_write(const char* str, size_t len) {
    for (size_t i = 0; i < len; i++) {
        vga_putchar(str[i]);
    }
}

void vga_writestring(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    vga_write(str, len);
}
