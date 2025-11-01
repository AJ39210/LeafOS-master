.PHONY: all clean kernel initrd os_fs iso img test

CC := gcc
AS := nasm
LD := ld
OBJCOPY := objcopy

CFLAGS := -m32 -ffreestanding -O2 -Wall -Wextra -fno-exceptions
ASFLAGS := -f elf32 -g

BUILD_DIR := build
SRC_DIR := src
KERNEL_DIR := $(SRC_DIR)/kernel
SHELL_DIR := $(SRC_DIR)/shell
INITRD_DIR := $(SRC_DIR)/initrd

# Build artifacts
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
INITRD_CPIO := $(BUILD_DIR)/initrd.cpio
OS_FS_CPIO := $(BUILD_DIR)/os_fs.cpio
BOOTABLE_ISO := $(BUILD_DIR)/LeafOS-installer.iso
BOOTABLE_IMG := $(BUILD_DIR)/LeafOS-installer.qcow2
SHELL_BIN := $(BUILD_DIR)/shell

all: kernel initrd os_fs iso

# ==================== KERNEL BUILD ====================

KERNEL_SRC := $(wildcard $(KERNEL_DIR)/*.c)
KERNEL_ASM := $(KERNEL_DIR)/boot.asm
KERNEL_OBJS := $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/%.o,$(KERNEL_SRC))
KERNEL_OBJS += $(BUILD_DIR)/boot.o

kernel: $(KERNEL_ELF)

$(BUILD_DIR)/boot.o: $(KERNEL_ASM)
	@mkdir -p $(BUILD_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $< -I$(KERNEL_DIR)

$(KERNEL_ELF): $(KERNEL_OBJS)
	$(LD) -m elf_i386 -T $(KERNEL_DIR)/linker.ld -o $@ $(KERNEL_OBJS)
	@echo "Kernel built: $@"

# ==================== SHELL BUILD ====================

$(SHELL_BIN): $(SHELL_DIR)/shell.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -m32 -O2 -Wall -Wextra -o $@ $<
	@echo "Shell built: $@"

# ==================== INITRD BUILD ====================

initrd: $(INITRD_CPIO)

$(INITRD_CPIO): $(KERNEL_ELF)
	@mkdir -p $(BUILD_DIR)
	@bash build/create_initrd.sh
	@echo "Initrd created: $@"

# ==================== OS FILESYSTEM BUILD ====================

os_fs: $(OS_FS_CPIO)

$(OS_FS_CPIO): $(SHELL_BIN) $(KERNEL_ELF) $(INITRD_CPIO)
	@mkdir -p $(BUILD_DIR)
	@bash build/create_os_fs.sh
	@echo "OS filesystem created: $@"

# ==================== ISO BUILD ====================

iso: $(BOOTABLE_ISO)

$(BOOTABLE_ISO): $(KERNEL_ELF) $(INITRD_CPIO)
	@mkdir -p $(BUILD_DIR)
	@bash build/create_bootable_iso.sh
	@echo "Bootable ISO created: $@"

# ==================== IMG BUILD ====================

img: $(BOOTABLE_IMG)

$(BOOTABLE_IMG): $(KERNEL_ELF) $(INITRD_CPIO)
	@mkdir -p $(BUILD_DIR)
	@bash build/create_bootable_img.sh
	@echo "Bootable IMG created: $@"

# ==================== TESTING ====================

test-iso: iso
	qemu-system-i386 -m 256 -cdrom $(BOOTABLE_ISO) -serial stdio

test-img: img
	qemu-system-i386 -m 256 -hda $(BOOTABLE_IMG) -serial stdio

# ==================== CLEANUP ====================

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(INITRD_DIR)/bin/sh $(INITRD_DIR)/bin/init
	@echo "Cleaned build artifacts"

cleanall: clean
	@echo "Full clean completed"
