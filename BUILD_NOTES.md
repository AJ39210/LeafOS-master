# Build Notes for LeafOS

## Quick Start

```bash
# Install prerequisites (Ubuntu/Debian)
sudo apt-get install build-essential nasm grub-pc-bin grub-common xorriso qemu-system-x86

# Note: You'll need a cross-compiler for full build
# See "Cross-Compiler Setup" below

# Check available tools
make check-tools

# Build (once cross-compiler is ready)
make iso

# Test in QEMU
make run
```

## Cross-Compiler Setup

LeafOS requires a GCC cross-compiler targeting x86_64-elf. Your system's native compiler won't work because:
1. It targets your host OS (Linux, macOS, etc.)
2. It includes standard library headers incompatible with freestanding environments
3. It uses the wrong memory model

### Building a Cross-Compiler

Follow the comprehensive guide at: https://wiki.osdev.org/GCC_Cross-Compiler

Quick version:

```bash
export PREFIX="$HOME/opt/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

# Download binutils and gcc
# (See OSDev wiki for exact versions and build commands)

# Build binutils
cd build-binutils
../binutils-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

# Build gcc
cd build-gcc
../gcc-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
```

This typically takes 30-60 minutes.

## Alternative: Docker Build Environment

If you don't want to build a cross-compiler, use a Docker container:

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    nasm \
    grub-pc-bin \
    grub-common \
    xorriso \
    qemu-system-x86 \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Add pre-built cross-compiler or build it here
# (See OSDev wiki for pre-built toolchains)

WORKDIR /workspace
```

## Build Process Explained

### Stage 1: Assembly Files
- `boot/multiboot2.asm` → `build/boot/multiboot2.o`
- `kernel/arch/boot.asm` → `build/kernel/arch/boot.o`
- `kernel/arch/gdt_flush.asm` → `build/kernel/arch/gdt_flush.o`
- `kernel/arch/isr_stubs.asm` → `build/kernel/arch/isr_stubs.o`

### Stage 2: C Files
All `.c` files compiled with:
- `-ffreestanding` (no standard library)
- `-nostdlib` (don't link standard library)
- `-mno-red-zone` (required for x86_64 kernel)
- `-mcmodel=kernel` (higher-half kernel)
- `-mno-sse -mno-sse2` (no SSE in kernel)

### Stage 3: Linking
All object files linked using `linker.ld`:
- Entry point: `_start` (from boot.asm)
- .multiboot section at beginning (for GRUB)
- Kernel linked at 0xFFFFFFFF80100000 (higher-half)
- But loaded at 0x100000 physical (1MB)

### Stage 4: ISO Creation
```
iso/
├── boot/
│   ├── grub/
│   │   └── grub.cfg
│   └── kernel.bin
```

GRUB's `grub-mkrescue` creates a bootable ISO with:
- GRUB bootloader
- Our kernel
- GRUB configuration

## Troubleshooting

### "x86_64-elf-gcc: command not found"
You need to build/install the cross-compiler. See "Cross-Compiler Setup" above.

### "grub-mkrescue: command not found"
```bash
sudo apt-get install grub-pc-bin grub-common xorriso
```

### "multiboot2 header not found"
GRUB can't find the Multiboot2 header. Ensure:
1. `boot/multiboot2.asm` is being assembled
2. Linker places `.multiboot` section at beginning
3. Check `linker.ld` has `KEEP(*(.multiboot))`

### QEMU boots to GRUB but kernel doesn't load
- Check that `grub.cfg` points to correct kernel path
- Verify kernel.bin exists in ISO at `/boot/kernel.bin`
- Check Multiboot2 magic number and checksum

### Triple fault / Instant reboot in QEMU
- Enable debugging: `qemu-system-x86_64 -cdrom leafos.iso -d int -no-reboot -D qemu.log`
- Check `qemu.log` for exception information
- Common causes:
  - Stack not properly set up
  - Invalid page tables
  - Wrong GDT/IDT configuration

### Page fault at boot
- Check page table setup in `boot.asm`
- Verify kernel is identity-mapped for first 2MB
- Check higher-half mappings are correct

## Testing Without Cross-Compiler

If you just want to see the code structure without building:

```bash
# View all source files
find . -name "*.c" -o -name "*.h" -o -name "*.asm" | xargs cat

# Check file organization
tree -I 'build|iso'

# Validate assembly syntax (if NASM is installed)
nasm -f elf64 boot/multiboot2.asm -o /dev/null
```

## Next Steps After Build

Once you have a working build:

1. **Test in QEMU:**
   ```bash
   make run
   # Should see LeafOS boot messages
   ```

2. **Test on real hardware (optional):**
   ```bash
   sudo dd if=leafos.iso of=/dev/sdX bs=4M status=progress
   # Boot from USB
   ```

3. **Continue development:**
   - See `planning.md` for implementation roadmap
   - Start with Phase 3: Keyboard input
   - Each phase builds on previous ones

## Common Build Flags Explained

- `-ffreestanding`: Tells compiler we're not using standard library
- `-nostdlib`: Don't link against standard library
- `-mno-red-zone`: x86_64 ABI uses "red zone" below stack pointer, but kernel interrupts would corrupt it
- `-mcmodel=kernel`: Code can be located anywhere in memory (higher-half kernel)
- `-mno-sse -mno-sse2`: Disable SSE (requires special initialization)
- `-O2`: Optimize for speed (can also use `-Os` for size)
- `-g`: Include debug symbols for debugging with GDB

## Debugging with GDB

```bash
# Terminal 1: Start QEMU with GDB server
qemu-system-x86_64 -cdrom leafos.iso -s -S

# Terminal 2: Connect GDB
gdb build/kernel.bin
(gdb) target remote localhost:1234
(gdb) break kernel_main
(gdb) continue
```

## Resources

- **OSDev Wiki**: https://wiki.osdev.org/
- **Multiboot2 Spec**: https://www.gnu.org/software/grub/manual/multiboot2/
- **Intel Manuals**: https://www.intel.com/sdm
- **NASM Documentation**: https://www.nasm.us/docs.php
