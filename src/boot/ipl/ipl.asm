%define BOOTINFO_START_ADDR 0x8000
; boot info:
;   offset (size)
;   + 0x00 (1)     : drive no.
;   + 0x10 (2)     : # of system memory map entries
;   + 0x20 (24*n)  : system memory map entries

bits 16

section .ipl
global entry
entry:
    jmp     0x0000:main ; reset cs to 0x0000

main:
    ; setup the initial stack base to 0x7c00
    xor     ax, ax
    mov     ss, ax
    mov     sp, entry

    ; initialize other segment registers
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax

    cld

    ; save the boot drive no. to boot info region
    xor     ax, ax
    mov     al, dl
    mov     [BOOTINFO_START_ADDR], ax

    ; set display mode
%define VGA_COLOR_TEXT_80_25 0x03
    mov     ah, 0x00
    mov     al, VGA_COLOR_TEXT_80_25
    int     0x10

    ; print a welcome message
    mov     si, message
    call    printstr

    ; read drive parameters
    mov     dl, [BOOTINFO_START_ADDR]
    xor     ax, ax
    mov     ah, 0x08
    int     0x13
    jc      printerror

    ; save the drive parameters
    ; max sector no.: (cl&0x3f), sector no. is 1-origin
    xor     ax, ax
    mov     al, cl
    and     al, 0x3f ; lower 6 bits
    mov     [sectors], al
    ; max head no.: dh, heads no. is 0-origin
    mov     al, dh
    mov     [heads], dh
    ; max track no.: ((cl&0xc0)<<2 | ch), track no. is 0-origin
    xor     ax, ax
    shr     cl, 6
    mov     ah, cl
    mov     al, ch
    mov     [tracks], ax

    ; load the OS loader to 0x09000
%define LOADER_SEGMENT 0x0000
%define LOADER_OFFSET 0x9000
%define LOADER_SIZE_IN_SECTORS  0x0020
    mov     dl, [BOOTINFO_START_ADDR]
    mov     cx, LOADER_SIZE_IN_SECTORS
    mov     ax, 0x01
    mov     bx, LOADER_SEGMENT
    mov     es, bx
    mov     bx, LOADER_OFFSET
    call    readsectors

    ; jump to the OS loader
    jmp    LOADER_SEGMENT:LOADER_OFFSET

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

; convert a 16-bit integer to its ASCII code and write it to [es:di]
bin2ascii:
    push    bx
    mov     bx, ax
    shr     ax, 12
    call    bin2ascii.lsb4
    shr     ax, 8
    call    bin2ascii.lsb4
    shr     ax, 4
    call    bin2ascii.lsb4
    call    bin2ascii.lsb4
    pop     bx
    ret
bin2ascii.lsb4:
    and     al, 0x0f
    cmp     al, 0x0a
    sbb     al, 0x69 ; ???
    das
    stosb
    mov     ax, bx ; preserve the ax value
    ret

; print an error message with error code (at ax)
printerror:
    push    bx
    push    es
    push    si
    push    di
    xor     bx, bx
    mov     es, bx
    mov     di, errno
    call    bin2ascii
    mov     si, errmsg
    call    printstr
    pop     di
    pop     si
    pop     es
    pop     bx
    ret

; increment (Cylinder (track), Head, Sector) values
; initial values:
;   sector: bl
;   head:   cl
;   track:  dx
incchs:
    ; increment the sector no.
    inc     bl
    cmp     bl, [sectors]
    jbe     incchs.end
    ; increment the head no.
    mov     bl, 0x01 ; reset the sector no.
    inc     cl
    cmp     cl, [heads]
    jbe     incchs.end
    ; increment the track no.
    xor     cl, cl ; reset the head no.
    inc     dx
incchs.end:
    ret

readsectors:
    ; read sectors from a disk to a specified address
    ; params:
    ;   dl: drive no.
    ;   cx: no. of sectors to be read
    ;   ax: LBA of the first sector
    ;   [es:bx]: address of a buffer
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
    add     bx, 0x200
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
readsectors.end:
    mov     dx, [bp-0x08]
    mov     cx, [bp-0x06]
    mov     bx, [bp-0x04]
    mov     ax, [bp-0x02]
    mov     sp, bp
    pop     bp
    ret

readsector:
    ; params:
    ;   same as the BIOS service int 0x13, ah=0x02 other than ax
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
    ja      printerror
    jmp     readsector.retry
readsector.end:
    mov     ax, [bp-0x02]
    mov     sp, bp
    pop     bp
    ret

message:
    db      `Hello World! This is IPL.\r\n`, 0x00
errmsg:
    db      'Disk error: 0x'
errno:
    db      0x3d, 0x3d, 0x3d, 0x3d
    db      `\r\n`, 0x00

; global variables
sectors:
    resw    1
heads:
    resw    1
tracks:
    resw    1

    ;resb    0x1be-($-$$)
    ; end of the IPL code area

partitions:
    ;times   16 db 0x00
    ;times   16 db 0x00
    ;times   16 db 0x00
    ;times   16 db 0x00

    resb    0x1fe-($-$$)
signature:
    db      0x55, 0xaa
    ; end of MBR

