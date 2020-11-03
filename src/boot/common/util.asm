halt:
    hlt
    jmp     halt
printstr:
    push    ax
    push    bx
printstr.1:
    lodsb
    test    al, al
    jnz     putc
    pop     bx
    pop     ax
    ret
putc:
    mov     ah, 0x0e
    mov     bx, 0x15
    int     0x10
    jmp     printstr.1
