bits 32

extern entry64

section .text
global entry32
entry32:
    cli

    ; set segment registers
    mov     eax, (2<<3)         ; 32-bit data segment descriptor
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

    ; go to 64-bit world
enter_64:
    push    (3<<3)              ; 64-bit code segment descriptor
    push    entry64
    retf

halt:
    hlt
    jmp     halt

