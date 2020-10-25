OUTPUT_ARCH(i386:x86-64);
ENTRY(entry16)

SECTIONS {
    . = 0x9000;
    .text : {
        entry16.o(.text)
        *(.text)
    }
    .data : { *(.data) }
    .rodata : { *(.rodata) }
}
