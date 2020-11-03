OUTPUT_ARCH(i386:x86-64);
ENTRY(kentry)

SECTIONS {
    . = 0x10000;
    .text : {
        *(.entry.text)
        *(.text)
    }
    .data : { *(.data) }
    .rodata : { *(.rodata) }
}
