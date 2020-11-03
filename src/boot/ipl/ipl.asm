bits 16

%include "boot/bootinfo.inc"

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

    ; save the boot drive no. to the bootinfo region
    xor     ax, ax
    mov     al, dl
    mov     [BOOTINFO_DRIVE_NO_ADDR], ax

    ; set display mode
%define VGA_COLOR_TEXT_80_25 0x03
    mov     ah, 0x00
    mov     al, VGA_COLOR_TEXT_80_25
    int     0x10

    ; print a welcome message
    mov     si, message
    call    printstr

    ; read drive parameters
    mov     dl, [BOOTINFO_DRIVE_NO_ADDR]
    xor     ax, ax
    mov     ah, 0x08
    int     0x13
    jc      halt_error

    ; save the drive parameters to the bootinfo region
    ; max sector no.: (cl&0x3f), sector no. is 1-origin
    xor     ax, ax
    mov     al, cl
    and     al, 0x3f ; lower 6 bits
    mov     [BOOTINFO_SECTORS_ADDR], ax
    ; max head no.: dh, heads no. is 0-origin
    xor     ax, ax
    mov     al, dh
    mov     [BOOTINFO_HEADS_ADDR], ax
    ; max track no.: ((cl&0xc0)<<2 | ch), track no. is 0-origin
    xor     ax, ax
    shr     cl, 6
    mov     ah, cl
    mov     al, ch
    mov     [BOOTINFO_TRACKS_ADDR], ax

    ; load the OS loader
%define LOADER_SEGMENT 0x0000
%define LOADER_OFFSET 0x9000
%define LOADER_SIZE_IN_SECTORS  0x0020
    mov     dl, [BOOTINFO_DRIVE_NO_ADDR]
    mov     cx, LOADER_SIZE_IN_SECTORS
    mov     ax, 0x01
    mov     bx, LOADER_SEGMENT
    mov     es, bx
    mov     bx, LOADER_OFFSET
    call    readsectors
    jc      halt

    ; jump to the OS loader
    jmp    LOADER_SEGMENT:LOADER_OFFSET

%include "read_sector.asm"
%include "util.asm"

halt_error:
    mov     si, message2
    call    printstr
    jmp     halt

message:
    db      `Hello World! This is IPL.\r\n`, 0x00
message2:
    db      `Error occured.\r\n`, 0x00

    resb    0x1be-($-$$)
    ; end of the IPL code area

partitions:
    times   16 db 0x00
    times   16 db 0x00
    times   16 db 0x00
    times   16 db 0x00

signature:
    db      0x55, 0xaa
    ; end of MBR

