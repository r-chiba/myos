OUTPUT_ARCH(i386:x86-64);
ENTRY(entry)

SECTIONS {
    . = 0x7c00;
    .ipl : { ipl(.ipl) }
}
