; Multiboot2 boot.s for PulseOS
; - Passes Multiboot2 info pointer (ebx) to kernel_main(uint32_t mb2_addr)
; - Detects valid Multiboot2 boot
; - Shows error on VGA text if invalid

SECTION .multiboot2
align 8
multiboot2_header:
    dd 0xE85250D6                  ; Multiboot2 magic
    dd 0                           ; Architecture 0 (x86)
    dd multiboot2_end - multiboot2_header ; Header length
    dd -(0xE85250D6 + 0 + (multiboot2_end - multiboot2_header)) ; Checksum

    dw 0    ; type (end tag)
    dw 0    ; flags
    dd 8    ; size
multiboot2_end:

SECTION .text
global _start
extern kernel_main

_start:
    ; EAX: Multiboot2 magic, EBX: address of Multiboot2 info struct
    cmp eax, 0x36d76289
    jne .invalid_boot

    push ebx        ; Pass Multiboot2 info pointer
    call kernel_main

    cli
.hang:
    hlt
    jmp .hang

.invalid_boot:
    ; Show error message in VGA text mode
    mov edi, 0xB8000
    mov esi, invalid_msg
    mov ah, 0x4F    ; White on red
.print_loop:
    lodsb
    test al, al
    jz .print_done
    stosw
    jmp .print_loop
.print_done:
    cli
    hlt
    jmp .print_done

SECTION .data
invalid_msg: db "Invalid bootloader - not Multiboot2 compliant", 0