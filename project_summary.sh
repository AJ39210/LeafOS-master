#!/bin/bash
# LeafOS Project Summary Script
# Displays current project status and file statistics

echo "========================================="
echo "       LeafOS Project Summary"
echo "========================================="
echo ""

# Count files by type
echo "File Statistics:"
echo "  C source files:        $(find . -name "*.c" | wc -l)"
echo "  Header files:          $(find . -name "*.h" | wc -l)"
echo "  Assembly files:        $(find . -name "*.asm" | wc -l)"
echo "  Total source files:    $(find . -name "*.c" -o -name "*.h" -o -name "*.asm" | wc -l)"
echo ""

# Count lines of code
if command -v wc &> /dev/null; then
    C_LINES=$(find . -name "*.c" -exec cat {} \; | wc -l)
    H_LINES=$(find . -name "*.h" -exec cat {} \; | wc -l)
    ASM_LINES=$(find . -name "*.asm" -exec cat {} \; | wc -l)
    TOTAL_LINES=$((C_LINES + H_LINES + ASM_LINES))

    echo "Lines of Code:"
    echo "  C code:                $C_LINES"
    echo "  Headers:               $H_LINES"
    echo "  Assembly:              $ASM_LINES"
    echo "  Total:                 $TOTAL_LINES"
    echo ""
fi

# Directory structure
echo "Project Structure:"
find . -type d -not -path '*/\.*' | sort | sed 's|[^/]*/| |g'
echo ""

# Implementation status
echo "Implementation Status:"
echo ""
echo "  ✅ Phase 1: Boot and Basic Output"
echo "     - Multiboot2 bootloader support"
echo "     - 64-bit long mode transition"
echo "     - VGA text mode driver"
echo ""
echo "  ✅ Phase 2: Interrupts and Memory"
echo "     - GDT and IDT configured"
echo "     - Exception and IRQ handlers"
echo "     - Physical memory manager"
echo "     - Virtual memory manager"
echo "     - Kernel heap allocator"
echo ""
echo "  ⬜ Phase 3: Input and Timing (Not Started)"
echo "     - Keyboard driver"
echo "     - ACPI driver"
echo "     - Timer driver"
echo ""
echo "  ⬜ Phase 4: Process Management (Not Started)"
echo "  ⬜ Phase 5: Disk and File System (Not Started)"
echo "  ⬜ Phase 6: Standard Library (Not Started)"
echo "  ⬜ Phase 7: Userspace Applications (Not Started)"
echo "  ⬜ Phase 8: Installation System (Not Started)"
echo ""

# Check for build tools
echo "Build Environment Check:"
if command -v x86_64-elf-gcc &> /dev/null; then
    echo "  ✅ x86_64-elf-gcc:     $(x86_64-elf-gcc --version | head -n1)"
else
    echo "  ❌ x86_64-elf-gcc:     Not found (required for building)"
fi

if command -v nasm &> /dev/null; then
    echo "  ✅ NASM:               $(nasm -version)"
else
    echo "  ❌ NASM:               Not found (required for building)"
fi

if command -v grub-mkrescue &> /dev/null; then
    echo "  ✅ grub-mkrescue:      Available"
else
    echo "  ❌ grub-mkrescue:      Not found (required for ISO creation)"
fi

if command -v qemu-system-x86_64 &> /dev/null; then
    echo "  ✅ QEMU:               $(qemu-system-x86_64 --version | head -n1)"
else
    echo "  ⚠️  QEMU:               Not found (needed for testing)"
fi

echo ""
echo "Next Steps:"
echo "  1. Install cross-compiler (see BUILD_NOTES.md)"
echo "  2. Run 'make iso' to build bootable image"
echo "  3. Run 'make run' to test in QEMU"
echo "  4. Continue with Phase 3 implementation"
echo ""
echo "For more information:"
echo "  - README.md:        Project overview and features"
echo "  - STATUS.md:        Detailed implementation status"
echo "  - BUILD_NOTES.md:   Build instructions and troubleshooting"
echo "  - planning.md:      Complete implementation specification"
echo ""
echo "========================================="
