; Multiboot header for GRUB bootloader
; x86 32-bit multiboot specification

section .multiboot
    align 4

    ; Multiboot header
    MULTIBOOT_MAGIC equ 0x1BADB002
    MULTIBOOT_FLAGS equ 0x00010003  ; require memory info + page align
    MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM
    dd 0, 0, 0, 0, 0  ; reserved fields
    dd 0  ; graphics mode (linear = 0)
    dd 0, 0, 0  ; graphics parameters

section .bss
    align 16
    stack_bottom:
        resb 16384  ; 16KB stack
    stack_top:

section .text
    extern kmain

    global _start

    _start:
        ; Stack setup
        mov esp, stack_top

        ; Save multiboot info (EBX contains pointer to multiboot_info_t)
        push ebx

        ; Call kmain
        call kmain

        ; Halt if kmain returns
        cli
        hlt
        jmp $
