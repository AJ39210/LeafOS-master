#!/bin/bash

# Script to create initrd CPIO archive

set -e

BUILD_DIR="build"
INITRD_DIR="src/initrd"
OUTPUT="$BUILD_DIR/initrd.cpio"

# Check if BusyBox is available
if ! command -v busybox &> /dev/null; then
    echo "Error: busybox not found. Please install busybox."
    echo "On Debian/Ubuntu: sudo apt-get install busybox-static"
    exit 1
fi

# Copy BusyBox to initrd
echo "Copying BusyBox to initrd..."
mkdir -p "$INITRD_DIR/bin"
cp "$(which busybox)" "$INITRD_DIR/bin/busybox"

# Create symlink for sh
if [ -e "$INITRD_DIR/bin/sh" ]; then
    rm "$INITRD_DIR/bin/sh"
fi
ln -s busybox "$INITRD_DIR/bin/sh"

# Make init script executable
chmod +x "$INITRD_DIR/sbin/init"
chmod +x "$INITRD_DIR/usr/sbin/installer"

# Create CPIO archive
echo "Creating CPIO archive..."
cd "$INITRD_DIR"
find . | cpio -o -H newc > "../../$OUTPUT"
cd - > /dev/null

echo "Initrd created: $OUTPUT"
