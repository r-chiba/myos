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

    call    centry

halt:
    hlt
    jmp     halt

section .data

