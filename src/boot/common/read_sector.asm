; read sectors from a disk to a specified address
; params:
;   dl: drive no.
;   cx: no. of sectors to be read
;   ax: LBA of the first sector
;   [es:bx]: address of a buffer
readsectors:
    push    bp
    mov     bp, sp
    sub     sp, 0x12
    mov     [bp-0x02], ax
    mov     [bp-0x04], bx
    mov     [bp-0x06], cx
    mov     [bp-0x08], dl
    xor     ax, ax
    mov     [bp-0x0a], ax ; local var for a sector counter
    mov     [bp-0x0c], ax ; local var for the current track
    mov     [bp-0x0e], ax ; local var for the current head (used only 1B)
    inc     ax
    mov     [bp-0x10], ax ; local var for the current sector (used only 1B)
    mov     [bp-0x12], bx ; local var for the current buffer addr
readsectors.init:
    xor     ax, ax
    ; set up the initial CHS
    xor     bx, bx
    mov     bl, 0x01 ; sector
    xor     cx, cx   ; head
    xor     dx, dx   ; track
readsectors.init.1:
    cmp     ax, [bp-0x02]
    je      readsectors.init.2
    call    incchs
    inc     ax
    jmp     readsectors.init.1
readsectors.init.2:
    mov     [bp-0x10], bx
    mov     [bp-0x0e], cx
    mov     [bp-0x0c], dx
    mov     ax, [bp-0x10]
readsectors.read:
    mov     ax, [bp-0x0a]
    cmp     ax, [bp-0x06]
    je      readsectors.end
    inc     ax
    mov     [bp-0x0a], ax
    mov     ax, [bp-0x0c]
    mov     cl, ah
    shl     cl, 6
    or      cl, [bp-0x10]
    mov     ch, al
    mov     dl, [bp-0x08]
    mov     dh, [bp-0x0e]
    mov     bx, [bp-0x12]
    call    readsector
    jc      readsectors.error
    add     bx, 0x200
    test    bx, bx
    jnz     readsectors.read.1
    mov     bx, es
    add     bx, 0x1000
    mov     es, bx
    xor     bx, bx
readsectors.read.1
    mov     [bp-0x12], bx
    ; update the CHS
    mov     bx, [bp-0x10]
    mov     cx, [bp-0x0e]
    mov     dx, [bp-0x0c]
    call    incchs
    mov     [bp-0x10], bx
    mov     [bp-0x0e], cx
    mov     [bp-0x0c], dx
    jmp     readsectors.read
readsectors.error:
    stc
readsectors.end:
    mov     dx, [bp-0x08]
    mov     cx, [bp-0x06]
    mov     bx, [bp-0x04]
    mov     ax, [bp-0x02]
    mov     sp, bp
    pop     bp
    ret

; params:
;   same as the BIOS service int 0x13, ah=0x02 other than ax
readsector:
%define MAX_RETRY 5
    push    bp
    mov     bp, sp
    sub     sp, 0x06
    mov     [bp-0x02], ax
    xor     ax, ax
    mov     [bp-0x04], ax ; local var for a retry counter
    mov     [bp-0x06], ax ; local var for an error code
readsector.retry:
    mov     ah, 0x02
    mov     al, 0x01
    int     0x13
    jnc     readsector.end
    ; retry
    mov     [bp-0x06], ax ; save the error code
    mov     ax, [bp-0x04]
    inc     ax
    mov     [bp-0x04], ax
    cmp     ax, MAX_RETRY
    mov     ax, [bp-0x06] ; restore the error code
    ;ja      printerror
    ja      readsector.error
    jmp     readsector.retry
readsector.error:
    stc
readsector.end:
    mov     ax, [bp-0x02]
    mov     sp, bp
    pop     bp
    ret

; increment (Cylinder (track), Head, Sector) values
; initial values:
;   sector: bl
;   head:   cl
;   track:  dx
incchs:
    ; increment the sector no.
    inc     bl
    cmp     bl, [BOOTINFO_SECTORS_ADDR]
    jbe     incchs.end
    ; increment the head no.
    mov     bl, 0x01 ; reset the sector no.
    inc     cl
    cmp     cl, [BOOTINFO_HEADS_ADDR]
    jbe     incchs.end
    ; increment the track no.
    xor     cl, cl ; reset the head no.
    inc     dx
incchs.end:
    ret

