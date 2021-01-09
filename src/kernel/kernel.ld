OUTPUT_ARCH(i386:x86-64);
ENTRY(kmain)

SECTIONS {
    . = 0x10000;

    . = ALIGN(4096);
    _kernel_start = .;
    .text : {
        *(.text.entry)
        *(.text)
    }
    .data : { *(.data) }
    .rodata : { *(.rodata) }
    .bss : { 
        *(.bss)
        *(.COMMON)
    }

    /* global descriptor table */
    .gdt ALIGN(8) : {
        *(.gdt)
    }
    /* interrupt descriptor table */
    .idt ALIGN(8) : {
        *(.idt)
    }
    . = ALIGN(4096);
    _kernel_end = .;
}
