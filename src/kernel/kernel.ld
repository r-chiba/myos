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
    _kernel_tmp1 = .;
    .data : AT(ADDR(.data) - CONFIG_KERNEL_VBASE + CONFIG_KERNEL_PBASE)
    { 
        *(.data)
        . = ALIGN(CONFIG_PAGE_SIZE);
        PROVIDE_HIDDEN(_trampoline_start = .);
        *(.trampoline)
        PROVIDE_HIDDEN(_trampoline_end = .);
    }
    _kernel_tmp2 = .;
    .rodata : AT(ADDR(.rodata) - CONFIG_KERNEL_VBASE + CONFIG_KERNEL_PBASE)
    { 
        *(.rodata)
    }
    _kernel_tmp3 = .;
    .bss : AT(ADDR(.bss) - CONFIG_KERNEL_VBASE + CONFIG_KERNEL_PBASE)
           ALIGN(CONFIG_PAGE_SIZE)
    { 
        PROVIDE_HIDDEN(_bss_start = .);
        *(.bss)
        *(.COMMON)
        PROVIDE_HIDDEN(_bss_end = .);
    }
    _kernel_tmp4 = .;

    /* global descriptor table */
    .gdt : AT(ADDR(.gdt) - CONFIG_KERNEL_VBASE + CONFIG_KERNEL_PBASE)
           ALIGN(8)
    {
        *(.gdt)
    }
    _kernel_tmp5 = .;
    /* interrupt descriptor table */
    .idt : AT(ADDR(.idt) - CONFIG_KERNEL_VBASE + CONFIG_KERNEL_PBASE)
           ALIGN(8)
    {
        *(.idt)
    }
#if 0
    _kernel_tmp6 = .;
    .trampoline : AT(ADDR(.trampoline) - CONFIG_KERNEL_VBASE + CONFIG_KERNEL_PBASE)
                  ALIGN(CONFIG_PAGE_SIZE)
    { 
        PROVIDE_HIDDEN(_trampoline_start = .);
        *(.trampoline)
        PROVIDE_HIDDEN(_trampoline_end = .);
    }
#endif
    /* . = ALIGN(CONFIG_PAGE_SIZE); */
    PROVIDE_HIDDEN(_kernel_end = .);

    /DISCARD/ :
    {
        *(.comment)
    }
}
