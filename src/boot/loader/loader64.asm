bits 64

extern centry

section .text
global entry64
entry64:
    cli

    ; clear the screen
    mov     edi, 0xb8000
    mov     ax, 0x0f20
    mov     ecx, 80*25
    rep stosw

    ; reset cursor position
    mov     ax, 0x000f
    mov     dx, 0x3d4
    out     dx, ax
    mov     ax, 0x000e
    mov     dx, 0x3d4
    out     dx, ax

    ; go to kernel
    ;call    centry
    mov     rax, 0x10000
    call    rax
    ;push    (3<<3)              ; 64bit code segment descriptor
    ;push    0x10000
    ;db      0x48                ; REX.W prefix to make sure params for retf (lret) are 64-bit
    ;retf

halt:
    hlt
    jmp     halt

section .data

