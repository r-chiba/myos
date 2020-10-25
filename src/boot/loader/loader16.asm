%define BOOTINFO_START_ADDR 0x8000
%define BOOTINFO_SMAP_NENTRIES_ADDR (BOOTINFO_START_ADDR+0x10)
%define BOOTINFO_SMAP_START_ADDR (BOOTINFO_START_ADDR+0x20)
; boot info:
;   offset (size)
;   + 0x00 (1)     : drive no.
;   + 0x10 (2)     : # of system memory map entries
;   + 0x20 (24*n)  : system memory map entries

bits 16

extern entry32

section .text
global entry16
entry16:
    ; print a welcome message
    ;mov     si, message1
    ;call    printstr

    call    check_long_mode
    jc      halt_error

    call    enable_a20
    jc      halt_error

    call    read_smap
    jc      halt_error

    ; enable protected mode
enable_protected_mode:
    cli
    ; load GDT and IDT
    lgdt    [gdtr]
    lidt    [idtr]

    ; set CR0.PE (bit 0)
    mov     eax, cr0
    or      al, (1<<0)
    mov     cr0, eax

    ; go to 32bit world
enter_32bit:
    jmp     (1<<3):entry32

    jmp     halt

    ; long mode support check
    ; see https://wiki.osdev.org/Setting_Up_Long_Mode
check_long_mode:
    ; cpuid support check
    pushfd
    pop     eax
    mov     ecx, eax
    xor     eax, 1<<21         ; flip the ID bit in FLAGS
    push    eax
    popfd
    pushfd
    pop     eax
    push    ecx
    popfd                      ; restore FLAGS
    xor     eax, ecx           ; the ID bit can be flipped if cpuid is supported
    jz      check_long_mode.no ; cpuid is not supported

    ; long mode support check
    mov     eax, 0x80000000
    cpuid
    cmp     eax, 0x80000001
    jb      check_long_mode.no ; long mode is not supported

    mov     eax, 0x80000001
    cpuid
    test    edx, 1<<29
    jz      check_long_mode.no ; long mode is not supported
    ret
check_long_mode.no:
    stc
    ret

    ; enable A20 gate to access memory above 1 MiB
enable_a20:
    push    ax
    push    cx
    cli
    xor     cx, cx
enable_a20.1:
    dec     cx
    jz      enable_a20.error    ; retry 2^16 times
    in      al, 0x64
    test    al, 0x2
    jnz     enable_a20.1
    mov     al, 0xd1            ; Write command
    out     0x64, al
enable_a20.2:
    in      al, 0x64
    test    al, 0x2
    jnz     enable_a20.2
    mov     al, 0xdf            ; enable A20 gate
    out     0x60, al
    jmp     enable_a20.end
enable_a20.error:
    stc
enable_a20.end:  
    sti
    pop     cx
    pop     ax
    ret

    ; save system memory map to 0x8020
%define QUERY_SMAP_SIG 0x534d4150   ; "SMAP"
%define QUERY_SMAP_ENTRY_SIZE 0x18
read_smap:
    push    eax
    push    ebx
    push    ecx
    push    edx
    push    di
    push    si
    xor     ebx, ebx
    mov     di, BOOTINFO_SMAP_START_ADDR
    xor     si, si ; # of entries
read_smap.1:
    mov     eax, 0xe820
    mov     ecx, QUERY_SMAP_ENTRY_SIZE
    mov     edx, QUERY_SMAP_SIG
    int     15h
    jc      read_smap.error
    inc     si
    cmp     eax, QUERY_SMAP_SIG
    jne     read_smap.error
    test    ebx, ebx
    jz      read_smap.end ; last entry
    add     di, QUERY_SMAP_ENTRY_SIZE
    jmp     read_smap.1
read_smap.error:
    stc
read_smap.end:
    mov     [BOOTINFO_SMAP_NENTRIES_ADDR], si
    pop     si
    pop     di
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
    ret

halt_error:
    mov     si, message2
    call    printstr
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

section .data
message1:
    db      `Hello World! This is OS loader.\r\n`, 0x00
message2:
    db      `Error occured.\r\n`, 0x00
errmsg:
    db      'Disk error: 0x'
errno:
    db      0x3d, 0x3d, 0x3d, 0x3d
    db      `\r\n`, 0x00

align 16
gdtr:
    dw      gdt.end - gdt - 1
    dq      gdt
align 16
gdt: ; Global Descriptor Table
    dd      0x0, 0x0                                       ; NULL descriptor
    db      0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00 ; 32bit code segment
    db      0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xaf, 0x00 ; 64bit code segment
    db      0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00 ; 32bit data segment
gdt.end:

align 16
idtr:
    dw      0x0
    dq      0x0
