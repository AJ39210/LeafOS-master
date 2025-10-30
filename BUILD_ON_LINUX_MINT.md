# Building LeafOS on Linux Mint

## Quick Start (3 Steps)

### Step 1: Install Missing Tools (if needed)

```bash
# Install basic build tools
sudo apt-get update
sudo apt-get install -y build-essential nasm grub-pc-bin grub-common xorriso qemu-system-x86
```

### Step 2: Check if You Have the Cross-Compiler

```bash
# Check for the most important tool
which x86_64-elf-gcc
```

**If found**: Skip to Step 3!
**If NOT found**: You need to build it (see below)

### Step 3: Build and Test LeafOS

```bash
# Go to the LeafOS directory
cd /path/to/LeafOS-master

# Option A: Use the automated script
./check_and_build.sh

# Option B: Manual build
make clean
make iso
make run
```

That's it! LeafOS will boot in QEMU.

---

## Building the Cross-Compiler (Only if Step 2 Failed)

If `x86_64-elf-gcc` is not found, build it once (takes 30-60 minutes):

```bash
# Set up directories
export PREFIX="$HOME/opt/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

# Download sources
cd ~
wget https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz
tar xf binutils-2.40.tar.gz
tar xf gcc-13.2.0.tar.gz

# Build binutils (15-20 minutes)
mkdir build-binutils
cd build-binutils
../binutils-2.40/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install

# Build GCC (30-40 minutes)
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

# Verify
which x86_64-elf-gcc
x86_64-elf-gcc --version
```

Now go back to Step 3 above!

---

## What Each Command Does

### `make clean`
Removes old build files

### `make iso`
- Compiles all C and assembly files
- Links them into kernel.bin
- Creates a bootable ISO with GRUB

### `make run`
- Launches QEMU
- Boots the ISO
- Shows LeafOS running

---

## Expected Output

When you run `make run`, you should see:

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
...
```

The OS will then halt (as designed - no interactive features yet).

---

## Troubleshooting

### "x86_64-elf-gcc: command not found"
Build the cross-compiler (see section above)

### "nasm: command not found"
```bash
sudo apt-get install nasm
```

### "grub-mkrescue: command not found"
```bash
sudo apt-get install grub-pc-bin grub-common xorriso
```

### Build succeeds but QEMU doesn't start
```bash
sudo apt-get install qemu-system-x86
```

### QEMU shows only black screen
This is normal! The kernel is running but halted. Try:
```bash
qemu-system-x86_64 -cdrom leafos.iso -m 512M -serial stdio
```

---

## Testing on Real Hardware

⚠️ **Advanced users only!**

```bash
# Write to USB drive (replace /dev/sdX with your USB device!)
sudo dd if=leafos.iso of=/dev/sdX bs=4M status=progress
sync

# Boot from USB
# LeafOS will run on real hardware!
```

---

## Quick Commands Reference

```bash
# Check what you have
./check_and_build.sh

# Build manually
make iso

# Test in QEMU
make run

# Test with serial output
qemu-system-x86_64 -cdrom leafos.iso -m 512M -serial stdio

# Test with debugging
make debug

# Clean build
make clean

# Check build tools
make check-tools

# View project stats
./project_summary.sh
```

---

## Next Steps After Successful Boot

Once LeafOS boots successfully:

1. **Explore the code**: Read the source files to understand how it works
2. **Implement Phase 3**: Add keyboard, ACPI, and timer (see planning.md)
3. **Modify and rebuild**: Change code → `make clean && make iso && make run`
4. **Test on real hardware**: Burn to USB and boot a physical machine

---

## Need Help?

- **BUILD_NOTES.md**: Detailed build instructions
- **STATUS.md**: What's implemented
- **README.md**: Project overview
- **planning.md**: Complete implementation spec
