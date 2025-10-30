# LeafOS Makefile
# Build system for the entire operating system

# Toolchain
CC := x86_64-elf-gcc
AS := nasm
LD := x86_64-elf-ld
OBJCOPY := x86_64-elf-objcopy

# Directories
BUILD_DIR := build
KERNEL_DIR := kernel
ISO_DIR := iso
BOOT_DIR := boot

# Compiler and assembler flags
CFLAGS := -ffreestanding -nostdlib -mno-red-zone -mcmodel=kernel \
          -mno-sse -mno-sse2 -Wall -Wextra -O2 -g -I$(KERNEL_DIR)
ASFLAGS := -f elf64
LDFLAGS := -nostdlib -T linker.ld

# Kernel sources
KERNEL_C_SRC := $(shell find $(KERNEL_DIR) -name '*.c')
KERNEL_ASM_SRC := $(KERNEL_DIR)/arch/boot.asm $(BOOT_DIR)/multiboot2.asm

# Object files
KERNEL_C_OBJ := $(KERNEL_C_SRC:%.c=$(BUILD_DIR)/%.o)
KERNEL_ASM_OBJ := $(KERNEL_ASM_SRC:%.asm=$(BUILD_DIR)/%.o)
KERNEL_OBJ := $(KERNEL_C_OBJ) $(KERNEL_ASM_OBJ)

# Default target
.PHONY: all
all: iso

# Build kernel binary
.PHONY: kernel
kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(KERNEL_OBJ)
	@echo "Linking kernel..."
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "Kernel built: $@"

# Compile C files
$(BUILD_DIR)/%.o: %.c
	@echo "Compiling $<..."
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble ASM files
$(BUILD_DIR)/%.o: %.asm
	@echo "Assembling $<..."
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# Build bootable ISO
.PHONY: iso
iso: kernel
	@echo "Creating bootable ISO..."
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(BUILD_DIR)/kernel.bin $(ISO_DIR)/boot/kernel.bin
	@cp $(BOOT_DIR)/grub.cfg $(ISO_DIR)/boot/grub/grub.cfg
	@if command -v grub-mkrescue > /dev/null 2>&1; then \
		grub-mkrescue -o leafos.iso $(ISO_DIR) 2>/dev/null || \
		echo "Warning: grub-mkrescue failed. You may need to install grub-pc-bin and xorriso."; \
	else \
		echo "Warning: grub-mkrescue not found. Install GRUB tools to create bootable ISO."; \
	fi
	@echo "ISO created: leafos.iso"

# Run in QEMU
.PHONY: run
run: iso
	@echo "Starting QEMU..."
	@if [ -f leafos.iso ]; then \
		qemu-system-x86_64 -cdrom leafos.iso -m 512M -serial stdio -no-reboot || \
		echo "QEMU not found or failed to run."; \
	else \
		echo "ISO file not found. Build may have failed."; \
	fi

# Run in QEMU with debugging
.PHONY: debug
debug: iso
	@echo "Starting QEMU with debugging..."
	@if [ -f leafos.iso ]; then \
		qemu-system-x86_64 -cdrom leafos.iso -m 512M -serial stdio -no-reboot -d int -D qemu.log || \
		echo "QEMU not found or failed to run."; \
	else \
		echo "ISO file not found. Build may have failed."; \
	fi

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR) $(ISO_DIR) leafos.iso qemu.log
	@echo "Clean complete."

# Show help
.PHONY: help
help:
	@echo "LeafOS Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build everything and create bootable ISO (default)"
	@echo "  kernel   - Build only the kernel binary"
	@echo "  iso      - Create bootable ISO image"
	@echo "  run      - Build and run in QEMU"
	@echo "  debug    - Build and run in QEMU with debugging"
	@echo "  clean    - Remove all build artifacts"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Requirements:"
	@echo "  - x86_64-elf cross-compiler (x86_64-elf-gcc)"
	@echo "  - NASM assembler"
	@echo "  - GRUB tools (grub-mkrescue, grub-pc-bin)"
	@echo "  - xorriso (for ISO creation)"
	@echo "  - QEMU (qemu-system-x86_64) for testing"

.PHONY: check-tools
check-tools:
	@echo "Checking for required tools..."
	@command -v $(CC) > /dev/null 2>&1 || echo "WARNING: $(CC) not found"
	@command -v $(AS) > /dev/null 2>&1 || echo "WARNING: $(AS) not found"
	@command -v $(LD) > /dev/null 2>&1 || echo "WARNING: $(LD) not found"
	@command -v grub-mkrescue > /dev/null 2>&1 || echo "WARNING: grub-mkrescue not found"
	@command -v xorriso > /dev/null 2>&1 || echo "WARNING: xorriso not found"
	@command -v qemu-system-x86_64 > /dev/null 2>&1 || echo "WARNING: qemu-system-x86_64 not found"
	@echo "Tool check complete."
