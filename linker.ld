/* Linker script for LeafOS kernel */

ENTRY(_start)

SECTIONS
{
    /* Kernel loaded at 1MB physical, but linked for higher half */
    . = 0xFFFFFFFF80100000;

    _kernel_start = .;

    /* Multiboot header must be at the beginning */
    .multiboot ALIGN(4K) : AT(ADDR(.multiboot) - 0xFFFFFFFF80000000)
    {
        KEEP(*(.multiboot))
    }

    /* Code section */
    .text ALIGN(4K) : AT(ADDR(.text) - 0xFFFFFFFF80000000)
    {
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xFFFFFFFF80000000)
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialized data */
    .data ALIGN(4K) : AT(ADDR(.data) - 0xFFFFFFFF80000000)
    {
        *(.data)
        *(.data.*)
    }

    /* Uninitialized data (BSS) */
    .bss ALIGN(4K) : AT(ADDR(.bss) - 0xFFFFFFFF80000000)
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    _kernel_end = .;

    /* Discard unnecessary sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.*)
    }
}
