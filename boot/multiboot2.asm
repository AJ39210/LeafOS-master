; Multiboot2 header for LeafOS kernel
; Allows GRUB to recognize and boot the kernel

section .multiboot
align 8

multiboot_header_start:
    dd 0xE85250D6                                      ; Magic number (Multiboot2)
    dd 0                                                ; Architecture (i386, covers x86_64)
    dd multiboot_header_end - multiboot_header_start   ; Header length
    dd -(0xE85250D6 + 0 + (multiboot_header_end - multiboot_header_start)) ; Checksum

    ; Information request tag
    align 8
information_request_tag_start:
    dw 1                            ; Type: information request
    dw 0                            ; Flags
    dd information_request_tag_end - information_request_tag_start  ; Size
    dd 4                            ; Request memory map
    dd 5                            ; Request boot command line
    dd 8                            ; Request framebuffer info
information_request_tag_end:

    ; Module alignment tag
    align 8
module_alignment_tag_start:
    dw 6                            ; Type: module alignment
    dw 0                            ; Flags
    dd module_alignment_tag_end - module_alignment_tag_start  ; Size
module_alignment_tag_end:

    ; End tag (required)
    align 8
    dw 0                            ; Type: end
    dw 0                            ; Flags
    dd 8                            ; Size

multiboot_header_end:
