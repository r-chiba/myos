bits 32

extern entry64

section .text
global entry32
entry32:
    cli

    ; disable PIC
    mov     al, 0xff
    out     0x21, al
    mov     al, 0xff
    out     0xa1, al

    ; set segment registers
    mov     eax, (3<<3)         ; 32bit data segment descriptor
    mov     ss, eax
    mov     ds, eax
    mov     es, eax
    mov     fs, eax
    mov     gs, eax

    ; set the stack pointer
    mov     eax, 0x7c00
    mov     esp, eax

    ; enable page address extension
enable_pae:
    ; set CR4.PAE (bit 5)
    mov     eax, cr4
    or      eax, (1<<5)
    mov     cr4, eax

    ; setup a page table which maps 4GiB straightly
    ; use 2MiB-page
    ; need 6 tables to map 4GiB (1 PML4, 1 PDPT, 4 PD, no PT)
setup_pt:
%define PAGE_TABLE_START_ADDR 0x00070000
    mov     ebx, PAGE_TABLE_START_ADDR
    ; initialize the table region with zero
    mov     edi, ebx
    xor     eax, eax
    mov     ecx, 6*512*8/4      ; a table has 512 entries (entry size: 8B)
    rep stosd

    ; setup entries of the PML4 table (PML4E)
    lea     eax, [ebx+0x1000]   ; PDPT is at 0x00071000
    or      eax, 0x007          ; set attrs; Present, R/W, User access
    mov     [ebx], eax

    ; setup entries of the PDP table (PDPTE)
    lea     edi, [ebx+0x1000]   ; PDPT addr
    lea     eax, [ebx+0x2000]   ; first PDT is at 0x00072000
    or      eax, 0x007          ; set attrs; Present, R/W, User access
    mov     ecx, 4              ; 4 PDs
setup_pt.1:
    mov     [edi], eax
    add     edi, 8
    add     eax, 0x1000
    loop    setup_pt.1

    ; setup entries of the PD tables (PDE)
    lea     edi, [ebx+0x2000]
    mov     eax, 0x0            ; straight-map 4GiB region from 0x0
    or      eax, 0x083          ; set attrs; Present, R/W, Supervisor access, 2MiB page
    mov     ecx, (512*4)        ; 512 entries, 4 tables
setup_pt.2:
    mov     [edi], eax
    add     edi, 8
    add     eax, 0x00200000     ; 1 page frame maps 2MiB
    loop    setup_pt.2

    ; set cr3 to PML4 address
    mov     cr3, ebx
    
    ; enable long mode
enable_long_mode:
    ; set EFER.LME (bit 8)
    mov     ecx, 0xc0000080
    rdmsr
    or      eax, (1<<8)
    wrmsr

    ; set CR0.PG (bit 31)
    mov     eax, cr0
    or      eax, (1<<31)
    mov     cr0, eax

    ; go to 64bit world
enter_64:
    push    (2<<3)              ; 64bit code segment descriptor
    push    entry64
    retf

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

