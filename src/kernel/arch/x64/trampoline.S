#include <config.h>
#include <arch/x64/asm.h>

    /* a section which have text and data needed for trampoline of APs */
    .section .trampoline, "w", @progbits

ASMFUNC_START(ap_entry)
    /* 
     * memo:
     * I wrote some instructions not by mnemonic but by opcode directly
     * because it seems the assembler can't emit appropriate 16bit code
     */
    .code16
    cli
    cld

    /* set segment registers and the stack pointer */
    /* %cs is set to (relocated address >> 4) automatically */
    xor     %eax, %eax
    movw    %cs, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
	.byte	0xbc /* movw $(tmp_stack-ap_entry), %sp */
    .long   tmp_stack-ap_entry

    /* save the relocated address of .trampoline to %ebx */
    xor     %ebx, %ebx
    movw    %cs, %bx
    shll    $4, %ebx

    /* load GDT */
	.byte	0xb8 /* movw $(gdtr_trampoline-ap_entry), %ax */
    .long   gdtr_trampoline-ap_entry
    orl     %ebx, 2(%eax) /* add the relocated address to the gdtr base field */
    lgdt    (%eax)

    /* set CR0.PE (bit 0) */
    movl    %cr0, %eax
    orl     $(1<<0), %eax
    movl    %eax, %cr0

    /* go to 32-bit world */
    xorl    %eax, %eax
	.byte	0xb8 /* movw $(ap_entry32-ap_entry), %ax */
    .long   ap_entry32-ap_entry
    orl     %ebx, %eax
    pushw   $(1<<3)
    pushl   %eax
    lretl
ASMFUNC_END(ap_entry)

ASMFUNC_START(ap_entry32)
    .code32
    cli

    /* set segment registers and the stack pointer */
	leal	stack_trampoline-ap_entry(%ebx), %ecx
    movw    $(2<<3), %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss
    movl    (%ecx), %esp

    /* enable page address extension */
    /* set CR4.PAE (bit 5) */
    movl    %cr4, %eax
    orl     $(1<<5), %eax
    movl    %eax, %cr4

    /* set cr3 to the temporal page table */
	leal	pml4_trampoline-ap_entry(%ebx), %eax
    movl    (%eax), %eax
    movl    %eax, %cr3
    
    /* enable long mode */
    /* set EFER.LME (bit 8) */
    movl    $0xc0000080, %ecx
    rdmsr
    orl     $(1<<8), %eax
    wrmsr
    /* set CR0.PG (bit 31) */
    movl    %cr0, %eax
    orl     $(1<<31), %eax
    movl    %eax, %cr0

    /* go to 64-bit world */
	leal	ap_entry64-ap_entry(%ebx), %eax
    pushl   $(3<<3)
    pushl   %eax
    lretl
ASMFUNC_END(ap_entry32)

ASMFUNC_START(ap_entry64)
    .code64
    cli
	leaq	kentry_ap-ap_entry(%rbx), %rax
    movq    (%rax), %rax
    callq   *%rax
ASMFUNC_END(ap_entry64)

    .balign 8
gdtr_trampoline:
    .word   gdt_trampoline.end - gdt_trampoline - 1
    .long   gdt_trampoline - ap_entry /* need to add the relocated address of .trampoline */

    .balign 16
gdt_trampoline:
    .word   0x0000, 0x0000, 0x0000, 0x0000 /* null descriptor */
    .word   0xffff, 0x0000, 0x9a00, 0x00cf /* 32-bit text descriptor */
    .word   0xffff, 0x0000, 0x9200, 0x00cf /* 32-bit data descriptor */
    .word   0xffff, 0x0000, 0x9a00, 0x00af /* 64-bit text descriptor */
    .word   0xffff, 0x0000, 0x9200, 0x00af /* 64-bit data descriptor */
gdt_trampoline.end:

/* temporal page table for trampoline */
/* will be assigned by the BSP */
    .globl pml4_trampoline
    .balign 8
pml4_trampoline:
    .long 0

/* kernel entrypoint for APs */
/* will be assigned by the BSP */
    .globl kentry_ap
    .balign 8
kentry_ap:
    .long 0

/* AP kernel stack */
/* will be assigned by the BSP */
    .globl stack_trampoline
    .balign 8
stack_trampoline:
    .long 0

/* temporal stack for trampoline 16-bit code */
    .balign 16
tmp_stack_end:
	.space	0x100
tmp_stack:

