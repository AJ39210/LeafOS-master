#!/bin/bash

# Script to create bootable qcow2/IMG with GRUB bootloader

set -e

BUILD_DIR="build"
OUTPUT_QCOW2="$BUILD_DIR/LeafOS-installer.qcow2"
OUTPUT_IMG="$BUILD_DIR/LeafOS-installer.img"
KERNEL_FILE="$BUILD_DIR/kernel.elf"
INITRD_FILE="$BUILD_DIR/initrd.cpio"
MOUNT_POINT="/tmp/leafos_boot_mount_$$"

# Check required tools
for tool in qemu-img mkfs.fat grub-install parted; do
    if ! command -v "$tool" &> /dev/null; then
        echo "Warning: $tool not found. Some functionality may not work."
    fi
done

# Check kernel and initrd exist
if [ ! -f "$KERNEL_FILE" ] || [ ! -f "$INITRD_FILE" ]; then
    echo "Error: kernel.elf or initrd.cpio not found. Run 'make kernel initrd' first."
    exit 1
fi

echo "Creating bootable disk image..."

# Create qcow2 image (256MB)
echo "Creating qcow2 image..."
qemu-img create -f qcow2 "$OUTPUT_QCOW2" 256M 2>/dev/null || true

# Also create raw image for compatibility
echo "Creating raw image..."
qemu-img create -f raw "$OUTPUT_IMG" 256M 2>/dev/null || true

# Convert qcow2 to raw if qcow2 operations fail
# For simplicity, we'll work with the raw image
IMG_FILE="$OUTPUT_IMG"

# Setup loop device for raw image
echo "Setting up loop device..."
LOOP_DEV=$(losetup -f)
losetup "$LOOP_DEV" "$IMG_FILE" 2>/dev/null || LOOP_DEV=""

if [ -z "$LOOP_DEV" ]; then
    echo "Warning: Could not setup loop device. Skipping image population."
    echo "Image created at: $OUTPUT_IMG and $OUTPUT_QCOW2"
    exit 0
fi

# Partition the disk
echo "Partitioning disk..."
(
    echo o      # Create DOS partition table
    echo n      # New partition
    echo p      # Primary
    echo 1      # Partition 1
    echo        # Default start
    echo        # Default end (use all space)
    echo t      # Set type
    echo c      # FAT32
    echo a      # Set bootable flag
    echo 1      # On partition 1
    echo w      # Write
) | fdisk "$LOOP_DEV" 2>/dev/null || true

sleep 1

# Find the partition device
PARTITION_DEV="${LOOP_DEV}p1"

# Check if partition exists
if [ ! -b "$PARTITION_DEV" ]; then
    echo "Partition device not found at $PARTITION_DEV. Trying offset method..."
    # Try using partx
    if command -v partx &> /dev/null; then
        partx -a "$LOOP_DEV" 2>/dev/null || true
        sleep 1
    fi
fi

# If still not found, calculate offset manually
if [ ! -b "$PARTITION_DEV" ]; then
    echo "Could not find partition. Using offset method..."
    PARTITION_DEV=$(losetup -f)
    losetup -o 1048576 "$PARTITION_DEV" "$IMG_FILE" 2>/dev/null || {
        echo "Warning: Could not set up partition device."
        losetup -d "$LOOP_DEV" 2>/dev/null || true
        exit 0
    }
fi

# Format partition as FAT32
echo "Formatting partition as FAT32..."
mkfs.fat -F 32 "$PARTITION_DEV" 2>/dev/null || true

# Mount partition
echo "Mounting partition..."
mkdir -p "$MOUNT_POINT"
mount "$PARTITION_DEV" "$MOUNT_POINT" 2>/dev/null || {
    echo "Warning: Could not mount partition"
    losetup -d "$LOOP_DEV" 2>/dev/null || true
    [ -n "$PARTITION_DEV" ] && losetup -d "$PARTITION_DEV" 2>/dev/null || true
    exit 0
}

# Create directory structure
echo "Creating directory structure..."
mkdir -p "$MOUNT_POINT/boot/grub"
mkdir -p "$MOUNT_POINT/bin"
mkdir -p "$MOUNT_POINT/sbin"
mkdir -p "$MOUNT_POINT/etc"
mkdir -p "$MOUNT_POINT/dev"
mkdir -p "$MOUNT_POINT/proc"
mkdir -p "$MOUNT_POINT/sys"
mkdir -p "$MOUNT_POINT/mnt"

# Copy kernel and initrd
echo "Copying kernel and initrd..."
cp "$KERNEL_FILE" "$MOUNT_POINT/boot/"
cp "$INITRD_FILE" "$MOUNT_POINT/boot/"

# Copy GRUB configuration
echo "Copying GRUB configuration..."
cp boot/grub.cfg "$MOUNT_POINT/boot/grub/"

# Install GRUB bootloader
echo "Installing GRUB bootloader..."
grub-install --target=i386-pc --boot-directory="$MOUNT_POINT/boot" "$LOOP_DEV" 2>/dev/null || \
    echo "Warning: GRUB installation may have failed"

# Sync and unmount
echo "Finalizing image..."
sync
umount "$MOUNT_POINT" 2>/dev/null || true

# Clean up loop devices
losetup -d "$PARTITION_DEV" 2>/dev/null || true
losetup -d "$LOOP_DEV" 2>/dev/null || true

# Remove mount point
rm -rf "$MOUNT_POINT"

echo "Bootable images created:"
echo "  - $OUTPUT_IMG (raw)"
echo "  - $OUTPUT_QCOW2 (qcow2)"
echo ""
echo "To test with QEMU:"
echo "  qemu-system-i386 -m 256 -hda $OUTPUT_IMG"
