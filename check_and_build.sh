#!/bin/bash
# LeafOS Build and Test Script for Linux Mint

echo "========================================"
echo "   LeafOS Build Check"
echo "========================================"
echo ""

# Check for required tools
MISSING_TOOLS=0

echo "Checking for required tools..."
echo ""

# Check for cross-compiler (most important)
if command -v x86_64-elf-gcc &> /dev/null; then
    echo "✅ x86_64-elf-gcc: $(x86_64-elf-gcc --version | head -n1)"
else
    echo "❌ x86_64-elf-gcc: NOT FOUND"
    echo "   This is the cross-compiler. You need to build it first."
    MISSING_TOOLS=1
fi

# Check for NASM
if command -v nasm &> /dev/null; then
    echo "✅ nasm: $(nasm -version)"
else
    echo "❌ nasm: NOT FOUND"
    echo "   Install with: sudo apt-get install nasm"
    MISSING_TOOLS=1
fi

# Check for make
if command -v make &> /dev/null; then
    echo "✅ make: $(make --version | head -n1)"
else
    echo "❌ make: NOT FOUND"
    echo "   Install with: sudo apt-get install build-essential"
    MISSING_TOOLS=1
fi

# Check for GRUB tools
if command -v grub-mkrescue &> /dev/null; then
    echo "✅ grub-mkrescue: Available"
else
    echo "❌ grub-mkrescue: NOT FOUND"
    echo "   Install with: sudo apt-get install grub-pc-bin grub-common xorriso"
    MISSING_TOOLS=1
fi

# Check for QEMU (optional but recommended)
if command -v qemu-system-x86_64 &> /dev/null; then
    echo "✅ qemu-system-x86_64: $(qemu-system-x86_64 --version | head -n1)"
else
    echo "⚠️  qemu-system-x86_64: NOT FOUND (optional, but needed for testing)"
    echo "   Install with: sudo apt-get install qemu-system-x86"
fi

echo ""
echo "========================================"

if [ $MISSING_TOOLS -eq 1 ]; then
    echo ""
    echo "❌ MISSING TOOLS DETECTED"
    echo ""
    echo "You need to install missing tools first."
    echo ""
    echo "Quick install (for most tools):"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install build-essential nasm grub-pc-bin grub-common xorriso qemu-system-x86"
    echo ""
    echo "For x86_64-elf-gcc cross-compiler, see BUILD_NOTES.md"
    echo "or run: ./install_cross_compiler.sh (if available)"
    echo ""
    exit 1
fi

echo ""
echo "✅ ALL TOOLS FOUND!"
echo ""
echo "Ready to build LeafOS!"
echo ""
echo "========================================"
echo ""

# Offer to build
read -p "Build LeafOS now? (y/n) " -n 1 -r
echo ""

if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo ""
    echo "Building LeafOS..."
    echo ""

    # Clean previous builds
    make clean

    # Build the ISO
    echo "Creating bootable ISO..."
    if make iso; then
        echo ""
        echo "✅ BUILD SUCCESSFUL!"
        echo ""
        echo "ISO created: leafos.iso"
        echo ""
        ls -lh leafos.iso
        echo ""

        # Offer to test in QEMU
        if command -v qemu-system-x86_64 &> /dev/null; then
            read -p "Test in QEMU now? (y/n) " -n 1 -r
            echo ""
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                echo ""
                echo "Starting QEMU..."
                echo "Press Ctrl+C to exit"
                echo ""
                sleep 2
                qemu-system-x86_64 -cdrom leafos.iso -m 512M
            else
                echo ""
                echo "To test later, run: make run"
                echo "Or manually: qemu-system-x86_64 -cdrom leafos.iso -m 512M"
            fi
        else
            echo "QEMU not installed. To test:"
            echo "  1. Install QEMU: sudo apt-get install qemu-system-x86"
            echo "  2. Run: make run"
        fi
    else
        echo ""
        echo "❌ BUILD FAILED"
        echo ""
        echo "Check the error messages above."
        echo "Common issues:"
        echo "  - Cross-compiler not in PATH"
        echo "  - Missing source files"
        echo "  - Syntax errors in code"
        echo ""
        exit 1
    fi
else
    echo ""
    echo "Skipped build. To build later:"
    echo "  make clean"
    echo "  make iso"
    echo "  make run"
    echo ""
fi
