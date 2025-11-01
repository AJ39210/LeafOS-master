#!/bin/bash

# Script to create OS filesystem CPIO archive

set -e

BUILD_DIR="build"
KERNEL_DIR="src/kernel"
SHELL_DIR="src/shell"
OUTPUT="$BUILD_DIR/os_fs.cpio"
STAGING="$BUILD_DIR/os_fs_staging"

echo "Creating OS filesystem..."

# Create staging directory
rm -rf "$STAGING"
mkdir -p "$STAGING"

# Create directory structure
mkdir -p "$STAGING/boot"
mkdir -p "$STAGING/bin"
mkdir -p "$STAGING/sbin"
mkdir -p "$STAGING/etc"
mkdir -p "$STAGING/dev"
mkdir -p "$STAGING/proc"
mkdir -p "$STAGING/sys"
mkdir -p "$STAGING/mnt"

# Copy kernel
if [ -f "$BUILD_DIR/kernel.elf" ]; then
    cp "$BUILD_DIR/kernel.elf" "$STAGING/boot/"
fi

# Copy shell binary
if [ -f "$BUILD_DIR/shell" ]; then
    cp "$BUILD_DIR/shell" "$STAGING/bin/sh"
fi

# Copy BusyBox
if command -v busybox &> /dev/null; then
    cp "$(which busybox)" "$STAGING/bin/busybox"
fi

# Create init script for installed OS
cat > "$STAGING/sbin/init" << 'EOF'
#!/bin/sh

# LeafOS OS Init Script
# Runs as PID 1 in the installed OS

echo "LeafOS Init Starting..."

# Mount kernel filesystems
mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t devtmpfs devtmpfs /dev

# Set up PATH
export PATH=/bin:/sbin:/usr/bin:/usr/sbin
export TERM=linux

# Set hostname
hostname leafos

# Start shell
exec /bin/sh
EOF

chmod +x "$STAGING/sbin/init"

# Create inittab
cat > "$STAGING/etc/inittab" << 'EOF'
# LeafOS Init Configuration
::sysinit:/sbin/init
::ctrlaltdel:/bin/reboot
EOF

# Create fstab
cat > "$STAGING/etc/fstab" << 'EOF'
proc    /proc   proc    defaults    0   0
sysfs   /sys    sysfs   defaults    0   0
EOF

# Create CPIO archive
echo "Creating CPIO archive..."
cd "$STAGING"
find . | cpio -o -H newc > "../../$OUTPUT"
cd - > /dev/null

# Clean up staging
rm -rf "$STAGING"

echo "OS filesystem created: $OUTPUT"
