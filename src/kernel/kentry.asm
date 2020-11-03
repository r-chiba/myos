bits 64

extern kmain

section .entry.text
global kentry
kentry:
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

    call    kmain

halt:
    hlt
    jmp     halt

section .data

