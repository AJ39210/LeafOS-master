#!/bin/bash

# Script to create bootable ISO using GRUB

set -e

BUILD_DIR="build"
OUTPUT_ISO="$BUILD_DIR/LeafOS-installer.iso"
KERNEL_FILE="$BUILD_DIR/kernel.elf"
INITRD_FILE="$BUILD_DIR/initrd.cpio"
ISO_STAGING="$BUILD_DIR/iso_staging"

# Check required tools
if ! command -v grub-mkrescue &> /dev/null; then
    echo "Error: grub-mkrescue not found. Please install grub2-tools."
    exit 1
fi

# Check kernel and initrd exist
if [ ! -f "$KERNEL_FILE" ] || [ ! -f "$INITRD_FILE" ]; then
    echo "Error: kernel.elf or initrd.cpio not found. Run 'make kernel initrd' first."
    exit 1
fi

echo "Creating bootable ISO..."

# Create staging directory
rm -rf "$ISO_STAGING"
mkdir -p "$ISO_STAGING/boot/grub"

# Copy kernel, initrd, and GRUB config
cp "$KERNEL_FILE" "$ISO_STAGING/boot/"
cp "$INITRD_FILE" "$ISO_STAGING/boot/"
cp boot/grub.cfg "$ISO_STAGING/boot/grub/"

# Create ISO using grub-mkrescue
echo "Generating ISO with GRUB..."
grub-mkrescue -o "$OUTPUT_ISO" "$ISO_STAGING" 2>/dev/null || {
    echo "Error: Failed to create ISO"
    rm -rf "$ISO_STAGING"
    exit 1
}

# Clean up staging
rm -rf "$ISO_STAGING"

echo "Bootable ISO created: $OUTPUT_ISO"
echo ""
echo "To test with QEMU:"
echo "  qemu-system-i386 -m 256 -cdrom $OUTPUT_ISO"
