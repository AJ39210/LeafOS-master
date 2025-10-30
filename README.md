# LeafOS

A custom operating system built from scratch for x86_64 architecture, implementing core OS functionality without networking, audio, or GUI (shell-only interface).

## Project Overview

LeafOS is a monolithic kernel operating system designed to run on real hardware with the following features:

### Implemented Features (Phase 1-2)

✅ **Phase 1: Boot and Basic Output**
- Multiboot2 bootloader support (GRUB)
- 64-bit long mode transition
- VGA text mode driver (80x25)
- Kernel initialization and early boot

✅ **Phase 2: Interrupts and Memory**
- Global Descriptor Table (GDT) for x86_64
- Interrupt Descriptor Table (IDT) with 256 entries
- Exception handlers (CPU exceptions 0-31)
- IRQ handlers (hardware interrupts 32-47)
- PIC (Programmable Interrupt Controller) remapping
- Physical Memory Manager (PMM) with bitmap allocation
- Virtual Memory Manager (VMM) with paging support
- Kernel heap allocator (kmalloc/kfree)

### Planned Features (Phase 3-8)

⬜ **Phase 3: Input and Timing**
- PS/2 Keyboard driver
- ACPI driver for hardware discovery and power management
- Timer interrupt (IRQ0) for scheduling

⬜ **Phase 4: Process Management**
- Process Control Blocks (PCB)
- Round-robin scheduler with 10ms time slices
- System call interface (SYSCALL instruction)
- Context switching

⬜ **Phase 5: Disk and File System**
- ATA PIO disk driver
- FAT32 file system implementation
- File operations (open, read, write, close, create, delete)

⬜ **Phase 6: Standard Library**
- printf/sprintf for formatted output
- String manipulation functions
- Userspace memory allocation

⬜ **Phase 7: Userspace Applications**
- Shell with built-in commands
- Calculator (integer arithmetic)
- Notepad (text editor with save)
- Scheduler viewer (process list display)

⬜ **Phase 8: Installation System**
- Two-stage installation (Live ISO → Installed OS)
- Disk partitioning and formatting
- GRUB installation

## Architecture

- **Target:** x86_64 (64-bit)
- **Language:** C (kernel and userspace) + Assembly (boot and low-level)
- **Bootloader:** GRUB with Multiboot2
- **Kernel Type:** Monolithic
- **File System:** FAT32
- **Memory Layout:**
  - User space: 0x0000000000000000 - 0x00007FFFFFFFFFFF (128TB)
  - Kernel space: 0xFFFF800000000000 - 0xFFFFFFFFFFFFFFFF (128TB)
  - Kernel code: 0xFFFFFFFF80100000+
  - Kernel heap: 0xFFFFFFFF90000000+

## Directory Structure

```
LeafOS-master/
├── boot/                   # Bootloader configuration
│   ├── multiboot2.asm      # Multiboot2 header
│   └── grub.cfg            # GRUB configuration
├── kernel/                 # Kernel source code
│   ├── arch/               # Architecture-specific code
│   │   ├── boot.asm        # Early boot and long mode transition
│   │   ├── gdt.c/gdt_flush.asm   # Global Descriptor Table
│   │   ├── idt.c/isr_stubs.asm   # Interrupt Descriptor Table
│   │   └── interrupts.c    # Interrupt handlers
│   ├── drivers/            # Device drivers
│   │   └── vga.c           # VGA text mode driver
│   ├── mm/                 # Memory management
│   │   ├── pmm.c           # Physical memory manager
│   │   ├── vmm.c           # Virtual memory manager
│   │   └── heap.c          # Kernel heap allocator
│   ├── include/            # Header files
│   └── kernel.c            # Kernel entry point
├── libc/                   # Standard C library (planned)
├── userspace/              # User applications (planned)
├── installer/              # Installation system (planned)
├── linker.ld               # Linker script
├── Makefile                # Build system
└── README.md               # This file
```

## Building LeafOS

### Prerequisites

You need a cross-compiler toolchain for x86_64-elf target:

1. **GCC Cross-Compiler:**
   ```bash
   # On Ubuntu/Debian
   sudo apt-get install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo

   # Download and build cross-compiler
   # Follow: https://wiki.osdev.org/GCC_Cross-Compiler
   ```

2. **NASM Assembler:**
   ```bash
   sudo apt-get install nasm
   ```

3. **GRUB Tools:**
   ```bash
   sudo apt-get install grub-pc-bin grub-common xorriso
   ```

4. **QEMU (for testing):**
   ```bash
   sudo apt-get install qemu-system-x86
   ```

### Building

```bash
# Build kernel only
make kernel

# Build bootable ISO
make iso

# Build and run in QEMU
make run

# Build and run with debugging
make debug

# Clean build artifacts
make clean

# Check for required tools
make check-tools
```

### Build Output

- `build/kernel.bin` - Kernel binary
- `leafos.iso` - Bootable ISO image

## Running LeafOS

### In QEMU (Recommended for Testing)

```bash
# Standard boot
qemu-system-x86_64 -cdrom leafos.iso -m 512M

# With serial output
qemu-system-x86_64 -cdrom leafos.iso -m 512M -serial stdio

# With debugging
qemu-system-x86_64 -cdrom leafos.iso -m 512M -d int -no-reboot
```

### On Real Hardware

1. Write ISO to USB drive:
   ```bash
   sudo dd if=leafos.iso of=/dev/sdX bs=4M status=progress
   sync
   ```

2. Boot from USB drive
3. LeafOS will display boot messages and initialization progress

## Development Status

### Current Implementation

**Phase 1-2 Complete:**
- Kernel boots via GRUB Multiboot2
- Transitions to 64-bit long mode
- VGA text output operational
- GDT and IDT configured
- Exception and IRQ handling functional
- Physical and virtual memory management operational
- Kernel heap allocator working

**What You'll See When Booting:**
```
LeafOS Booting...
Multiboot2 magic verified
Parsing Multiboot2 info...
Boot loader: GRUB 2.xx
Memory map found

=== Phase 2: Interrupts and Memory ===
Initializing GDT...
Initializing IDT...
Initializing physical memory manager...
PMM initialized: Total memory detected
Initializing virtual memory manager...
VMM initialized
Initializing kernel heap...
Heap initialized
Enabling interrupts...

LeafOS kernel initialized successfully!

Phase 1-2 complete!
- [X] Phase 1: Boot and basic output
- [X] Phase 2: Interrupts and Memory

Remaining phases need implementation:
- [ ] Phase 3: Input and Timing (keyboard, ACPI, timer)
- [ ] Phase 4: Process Management (scheduler, syscalls)
- [ ] Phase 5: Disk and File System (ATA, FAT32)
- [ ] Phase 6: Standard Library (printf, malloc, strings)
- [ ] Phase 7: Userspace Apps (shell, calculator, notepad)
- [ ] Phase 8: Installation System
```

### Continuing Development

To implement remaining phases:

1. **Phase 3 (Input/Timing):**
   - Implement keyboard driver in `kernel/drivers/keyboard.c`
   - Add ACPI driver in `kernel/drivers/acpi.c`
   - Add timer driver in `kernel/arch/timer.c`

2. **Phase 4 (Processes):**
   - Create process structures in `kernel/proc/process.c`
   - Implement scheduler in `kernel/proc/scheduler.c`
   - Add system calls in `kernel/proc/syscall.c`

3. **Phase 5 (Disk/FS):**
   - Implement ATA driver in `kernel/drivers/disk.c`
   - Add FAT32 driver in `kernel/fs/fat32.c`

4. **Phases 6-8:**
   - Standard library in `libc/`
   - Userspace applications in `userspace/`
   - Installation system in `installer/`

## Technical Details

### Boot Process

1. BIOS/UEFI loads GRUB from boot device
2. GRUB reads `grub.cfg` and loads `kernel.bin`
3. GRUB transfers control via Multiboot2 protocol
4. Kernel enters at `_start` in `boot.asm` (32-bit protected mode)
5. Boot code sets up:
   - Stack (16KB)
   - GDT for 64-bit mode
   - Page tables (PML4, PDPT, PD) with identity mapping
   - Long mode enabled via EFER MSR
   - Paging enabled
6. Far jump to 64-bit code segment
7. `kernel_main()` called in C

### Memory Management

**Physical Memory Manager (PMM):**
- Bitmap-based allocation (1 bit per 4KB page)
- Initialized from Multiboot2 memory map
- Supports single and multiple page allocation
- Reserves kernel memory, VGA, BIOS regions

**Virtual Memory Manager (VMM):**
- 4-level paging (PML4 → PDPT → PD → PT)
- Higher-half kernel at 0xFFFFFFFF80000000
- Simplified implementation (relies on boot.asm identity mapping)
- Support for creating new address spaces

**Kernel Heap:**
- First-fit allocator with block headers
- Automatic coalescing of free blocks
- Supports kmalloc, kfree, krealloc
- Initial size: 1MB (expandable)

### Interrupt Handling

**Exceptions (0-31):**
- Division by zero, invalid opcode, general protection fault, etc.
- Custom handlers display error information and halt system
- Page fault handler (ISR 14) shows faulting address

**Hardware IRQs (32-47):**
- PIC remapped from 0-15 to 32-47
- IRQ0: Timer (for future scheduler)
- IRQ1: Keyboard (for future input)
- IRQ14/15: ATA disk (for future file I/O)

## Known Limitations

**By Design (Requirements):**
- No networking support
- No audio drivers
- No GUI (text mode only)
- No mouse support

**Current Implementation:**
- Single-core only (no SMP)
- No process management yet
- No file system yet
- No user/kernel separation enforcement
- Simplified virtual memory (basic paging)

## License

LeafOS is licensed under the GNU General Public License v3.0. See LICENSE file for details.

## References

- [OSDev Wiki](https://wiki.osdev.org/) - Comprehensive OS development resource
- [Multiboot2 Specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html)
- [Intel 64 and IA-32 Architectures Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [FAT32 Specification](https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system)

## Contributing

This is an educational OS project. To contribute:

1. Follow existing code style (4 spaces, K&R braces)
2. Test changes in QEMU before submitting
3. Document new features in code comments
4. Update this README with significant changes

## Authors

Built as a demonstration of low-level systems programming and OS development concepts.

---

**Note:** This is a work in progress. Phases 3-8 require implementation as outlined in `planning.md`.
