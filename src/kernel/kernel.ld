#include <config.h> /* this linker script is pre-processed by C pre-processor */
OUTPUT_ARCH(i386:x86-64);
ENTRY(kmain)

SECTIONS {
    . = CONFIG_KERNEL_VBASE;
    PROVIDE_HIDDEN(_kernel_start = .);

    .text : AT(ADDR(.text) - CONFIG_KERNEL_VBASE + CONFIG_KERNEL_PBASE)
    {
        *(.text.entry)
        *(.text)
    }
    .data : AT(ADDR(.data) - CONFIG_KERNEL_VBASE + CONFIG_KERNEL_PBASE)
    { 
        *(.data)
    }
    .rodata : AT(ADDR(.rodata) - CONFIG_KERNEL_VBASE + CONFIG_KERNEL_PBASE)
    { 
        *(.rodata)
    }
    .bss : AT(ADDR(.bss) - CONFIG_KERNEL_VBASE + CONFIG_KERNEL_PBASE)
           ALIGN(CONFIG_PAGE_SIZE)
    { 
        PROVIDE_HIDDEN(_bss_start = .);
        *(.bss)
        *(.COMMON)
        PROVIDE_HIDDEN(_bss_end = .);
    }

    /* global descriptor table */
    .gdt : AT(ADDR(.gdt) - CONFIG_KERNEL_VBASE + CONFIG_KERNEL_PBASE)
           ALIGN(8)
    {
        *(.gdt)
    }
    /* interrupt descriptor table */
    .idt : AT(ADDR(.idt) - CONFIG_KERNEL_VBASE + CONFIG_KERNEL_PBASE)
           ALIGN(8)
    {
        *(.idt)
    }
    . = ALIGN(CONFIG_PAGE_SIZE);
    PROVIDE_HIDDEN(_kernel_end = .);

    /DISCARD/ :
    {
        *(.comment)
    }
}
