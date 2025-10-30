; Early boot code for LeafOS kernel
; Handles transition from 32-bit protected mode to 64-bit long mode

global _start
extern kernel_main

section .text
bits 32

_start:
    ; We arrive here from GRUB in 32-bit protected mode
    ; EAX contains Multiboot2 magic number (0x36d76289)
    ; EBX contains physical address of Multiboot2 info structure

    ; Set up stack
    mov esp, stack_top
    mov ebp, esp

    ; Save Multiboot2 info
    push ebx        ; Multiboot2 info structure pointer
    push eax        ; Multiboot2 magic number

    ; Verify Multiboot2 magic number
    cmp eax, 0x36d76289
    jne .no_multiboot

    ; Check for CPUID support
    call check_cpuid
    test eax, eax
    jz .no_cpuid

    ; Check for long mode support
    call check_long_mode
    test eax, eax
    jz .no_long_mode

    ; Set up paging for long mode
    call setup_page_tables

    ; Load GDT
    lgdt [gdt64.pointer]

    ; Enable PAE (Physical Address Extension)
    mov eax, cr4
    or eax, (1 << 5)    ; Set PAE bit
    mov cr4, eax

    ; Load PML4 address into CR3
    mov eax, pml4_table
    mov cr3, eax

    ; Enable long mode by setting EFER.LME bit
    mov ecx, 0xC0000080     ; EFER MSR
    rdmsr
    or eax, (1 << 8)        ; Set LME bit
    wrmsr

    ; Enable paging and protection
    mov eax, cr0
    or eax, (1 << 31)       ; Set PG bit (paging)
    or eax, (1 << 0)        ; Set PE bit (protection)
    mov cr0, eax

    ; Far jump to 64-bit code segment
    jmp gdt64.code:long_mode_start

.no_multiboot:
    mov al, 'M'
    jmp error

.no_cpuid:
    mov al, 'C'
    jmp error

.no_long_mode:
    mov al, 'L'
    jmp error

error:
    ; Print error character to screen (0xB8000)
    mov dword [0xB8000], 0x4F524F45     ; "ER" in red
    mov byte [0xB8004], al               ; Error code
    mov byte [0xB8005], 0x4F
    hlt

; Check if CPUID is supported
check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, (1 << 21)      ; Flip ID bit
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    cmp eax, ecx
    je .no_cpuid
    mov eax, 1
    ret
.no_cpuid:
    xor eax, eax
    ret

; Check if long mode is supported
check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    mov eax, 0x80000001
    cpuid
    test edx, (1 << 29)     ; Test LM bit
    jz .no_long_mode
    mov eax, 1
    ret
.no_long_mode:
    xor eax, eax
    ret

; Set up page tables for long mode
setup_page_tables:
    ; Zero out page tables
    mov edi, pml4_table
    mov ecx, 4096 * 3 / 4   ; 3 pages (PML4, PDPT, PD)
    xor eax, eax
    rep stosd

    ; Set up PML4 entry
    mov eax, pdpt_table
    or eax, 0b11            ; Present + writable
    mov [pml4_table], eax

    ; Set up PDPT entry
    mov eax, pd_table
    or eax, 0b11            ; Present + writable
    mov [pdpt_table], eax

    ; Identity map first 2MB using 2MB pages
    mov eax, 0b10000011     ; Present + writable + huge page
    mov [pd_table], eax

    ; Map kernel at higher half (0xFFFFFFFF80000000)
    ; This maps to physical address 0x00000000 (where we're loaded)
    mov eax, pd_table
    or eax, 0b11
    mov [pml4_table + 511 * 8], eax     ; Last entry in PML4

    mov eax, pdpt_table
    or eax, 0b11
    mov [pdpt_table + 510 * 8], eax     ; Entry 510 in PDPT

    ret

bits 64
long_mode_start:
    ; We're now in 64-bit long mode!

    ; Set up segment registers
    mov ax, gdt64.data
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Restore Multiboot2 info from stack
    pop rdi         ; Multiboot2 magic number (first argument)
    pop rsi         ; Multiboot2 info pointer (second argument)

    ; Call kernel main function
    call kernel_main

    ; If kernel_main returns, halt
.halt:
    cli
    hlt
    jmp .halt

; Global Descriptor Table for 64-bit mode
section .rodata
gdt64:
    dq 0                            ; Null descriptor
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53)  ; Code segment
.data: equ $ - gdt64
    dq (1<<44) | (1<<47)            ; Data segment
.pointer:
    dw $ - gdt64 - 1                ; Limit
    dq gdt64                        ; Base

; Reserve space for page tables (aligned to 4KB)
section .bss
align 4096
pml4_table:
    resb 4096
pdpt_table:
    resb 4096
pd_table:
    resb 4096

; Stack (16KB)
stack_bottom:
    resb 16384
stack_top:
