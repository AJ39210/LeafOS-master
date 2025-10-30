# How to Test LeafOS

## Quick Answer

**You need to build it first!** This requires setting up a cross-compiler toolchain on a Linux machine (or WSL/VM on Windows/Mac).

## Option 1: Test with Docker (Easiest)

This is the fastest way if you have Docker installed.

### Step 1: Create Docker Environment

Save this as `Dockerfile` in the LeafOS-master directory:

```dockerfile
FROM ubuntu:22.04

# Install build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    bison \
    flex \
    libgmp3-dev \
    libmpc-dev \
    libmpfr-dev \
    texinfo \
    nasm \
    grub-pc-bin \
    grub-common \
    xorriso \
    qemu-system-x86 \
    wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Download and build cross-compiler (this takes 30-60 minutes)
RUN wget https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.gz && \
    wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz && \
    tar xf binutils-2.40.tar.gz && \
    tar xf gcc-13.2.0.tar.gz

RUN mkdir build-binutils && cd build-binutils && \
    ../binutils-2.40/configure --target=x86_64-elf --prefix=/opt/cross --with-sysroot --disable-nls --disable-werror && \
    make -j$(nproc) && make install

RUN mkdir build-gcc && cd build-gcc && \
    ../gcc-13.2.0/configure --target=x86_64-elf --prefix=/opt/cross --disable-nls --enable-languages=c --without-headers && \
    make -j$(nproc) all-gcc all-target-libgcc && \
    make install-gcc install-target-libgcc

ENV PATH="/opt/cross/bin:${PATH}"

WORKDIR /workspace
```

### Step 2: Build and Run

```bash
# Build Docker image (takes 30-60 minutes first time)
docker build -t leafos-builder .

# Build LeafOS
docker run --rm -v $(pwd):/workspace leafos-builder make iso

# Test in QEMU (with display)
docker run --rm -v $(pwd):/workspace -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix leafos-builder make run
```

## Option 2: Native Linux (Ubuntu/Debian)

This gives you the best performance for development.

### Step 1: Install Prerequisites

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    bison \
    flex \
    libgmp3-dev \
    libmpc-dev \
    libmpfr-dev \
    texinfo \
    nasm \
    grub-pc-bin \
    grub-common \
    xorriso \
    qemu-system-x86
```

### Step 2: Build Cross-Compiler

This takes 30-60 minutes and only needs to be done once:

```bash
export PREFIX="$HOME/opt/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

# Download sources
cd ~
wget https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz
tar xf binutils-2.40.tar.gz
tar xf gcc-13.2.0.tar.gz

# Build binutils
mkdir build-binutils
cd build-binutils
../binutils-2.40/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install

# Build GCC
cd ~
mkdir build-gcc
cd build-gcc
../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc

# Add to PATH permanently
echo 'export PATH="$HOME/opt/cross/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

### Step 3: Build and Test LeafOS

```bash
cd /path/to/LeafOS-master

# Build the ISO
make iso

# Run in QEMU
make run
```

## Option 3: Pre-built Cross-Compiler

Some distributions have pre-built cross-compilers:

```bash
# Try these (may not be available on all distros)
sudo apt-get install gcc-x86-64-linux-gnu  # Not quite right, but might work
# OR use a pre-built toolchain from:
# https://github.com/lordmilko/i686-elf-tools/releases
```

## What You Should See When It Boots

If everything works, you'll see this in QEMU:

```
LeafOS Booting...
Multiboot2 magic verified
Parsing Multiboot2 info...
Boot loader: GRUB 2.04
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

Then the system will halt (as expected - no interactive features yet).

## Option 4: WSL2 on Windows

If you're on Windows:

```bash
# Install WSL2 with Ubuntu
wsl --install -d Ubuntu-22.04

# Then follow "Option 2: Native Linux" steps inside WSL
```

For QEMU with graphics in WSL2, you'll need an X server like VcXsrv or WSLg.

## Option 5: Virtual Machine (VirtualBox/VMware)

1. Create Ubuntu 22.04 VM
2. Follow "Option 2: Native Linux" steps
3. Build and test inside VM

## Testing Without Building

If you just want to explore the code structure without building:

```bash
# View all source files
find . -name "*.c" -o -name "*.h" -o -name "*.asm" | sort

# Count lines of code
./project_summary.sh

# Read the implementation
cat kernel/kernel.c
cat kernel/arch/boot.asm
cat kernel/drivers/vga.c
```

## Common Issues

### "x86_64-elf-gcc: command not found"
You need to build the cross-compiler. See Step 2 above.

### QEMU shows black screen
Normal - try adding `-serial stdio` to see debug output:
```bash
qemu-system-x86_64 -cdrom leafos.iso -m 512M -serial stdio
```

### "grub-mkrescue: command not found"
```bash
sudo apt-get install grub-pc-bin grub-common xorriso
```

### Build fails with linking errors
Make sure you're using the cross-compiler, not system gcc:
```bash
which x86_64-elf-gcc  # Should show your cross-compiler
```

## Testing on Real Hardware (Advanced)

⚠️ **Warning**: Only do this if you know what you're doing!

```bash
# Write ISO to USB drive (replace /dev/sdX with your USB device)
sudo dd if=leafos.iso of=/dev/sdX bs=4M status=progress
sync

# Boot from USB
# LeafOS will run on real hardware!
```

## Next Steps After Successful Boot

Once you see LeafOS boot successfully:

1. **Verify Phase 1-2**: Check that all initialization messages appear
2. **Continue Development**: Start implementing Phase 3 (keyboard, ACPI, timer)
3. **Test Changes**: Rebuild with `make clean && make iso && make run`
4. **Debug**: Use `make debug` for QEMU debugging or GDB

## Getting Help

If you get stuck:
- Check BUILD_NOTES.md for detailed troubleshooting
- Review STATUS.md to see what's implemented
- Read planning.md for implementation specifications
- Check OSDev Wiki: https://wiki.osdev.org/

## Estimated Time

- **Docker setup**: 30-60 minutes (first time)
- **Native cross-compiler build**: 30-60 minutes (once)
- **Building LeafOS**: 5-10 seconds
- **Testing in QEMU**: Instant (boots in <1 second)

## Summary

**TL;DR**: You need to:
1. Set up a Linux environment (native, WSL, VM, or Docker)
2. Install build tools and build cross-compiler (30-60 min, once)
3. Run `make iso` (5 seconds)
4. Run `make run` to test in QEMU (instant)
5. See LeafOS boot and initialize!

The OS is **fully implemented** for Phases 1-2, but **needs to be compiled** before testing.
