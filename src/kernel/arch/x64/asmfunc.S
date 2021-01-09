#include <arch/x64/asm.h>

    .text

/*
 * void halt(void);
 * halt this processor core
 */
ASMFUNC_START(halt)
1:
    hlt
    jmp     1b
ASMFUNC_END(halt)

/*
 * void disableInterrupts(void);
 * disable maskable interrupts
 */
ASMFUNC_START(disableInterrupts)
    cli
    ret
ASMFUNC_END(disableInterrupts)

/*
 * void enableInterrupts(void);
 * enable maskable interrupts
 */
ASMFUNC_START(enableInterrupts)
    sti
    ret
ASMFUNC_END(enableInterrupts)

/*
 * uint64_t readMsr(uint32_t addr);
 * read the value of a model-specific register specified by addr
 * arguments:
 *  %rdi: addr
 */
ASMFUNC_START(readMsr)
    movl    %edi, %ecx
    rdmsr
    shlq    $32, %rdx
    orq     %rdx, %rax
    ret
ASMFUNC_END(readMsr)

/*
 * void writeMsr(uint32_t addr, uint64_t val);
 * write val to a model-specific register specified by addr
 * arguments:
 *  %rdi: addr
 *  %rsi: val
 */
ASMFUNC_START(writeMsr)
    movl    %edi, %ecx
    movl    %esi, %eax
    shrq    $32, %rsi
    movl    %esi, %edx
    wrmsr
    ret
ASMFUNC_END(writeMsr)

/*
 * void cpuid(
 *          uint32_t func,
 *          uint32_t *eax,
 *          uint32_t *ebx,
 *          uint32_t *ecx,
 *          uint32_t *edx);
 * do the CPUID instruction
 * arguments:
 *  %rdi: CPUID function number
 *  %rsi: address of a variable load eax value of CPUID
 *  %rdx: address of a variable load ebx value of CPUID
 *  %rcx: address of a variable load ecx value of CPUID
 *  %r8: address of a variable load edx value of CPUID
 */
ASMFUNC_START(cpuid)
    movq    %rdx, %r9
    movq    %rcx, %r10
    movl    %edi, %eax
    cpuid
    movl    %eax, (%rsi)
    movl    %ebx, (%r9)
    movl    %ecx, (%r10)
    movl    %edx, (%r8)
    ret
ASMFUNC_END(cpuid)


/*
 * uint8_t in8(uint16_t port);
 * read a 1-byte value from an I/O port specified by port
 * arguments:
 *  %rdi: port
 */
ASMFUNC_START(in8)
    movw    %di, %dx
    inb     %dx, %al
    ret
ASMFUNC_END(in8)

/*
 * void out8(uint16_t port, uint8_t val);
 * write val to an I/O port specified by port
 * arguments:
 *  %rdi: port
 *  %rsi: val
 */
ASMFUNC_START(out8)
    movw    %di, %dx
    movq    %rsi, %rax
    outb    %al, %dx
    ret
ASMFUNC_END(out8)

/*
 * uint16_t in16(uint16_t port);
 * read a 2-byte value from an I/O port specified by port
 * arguments:
 *  %rdi: port
 */
ASMFUNC_START(in16)
    movw    %di, %dx
    inw     %dx, %ax
    ret
ASMFUNC_END(in16)

/*
 * void out16(uint16_t port, uint16_t val);
 * write val to an I/O port specified by port
 * arguments:
 *  %rdi: port
 *  %rsi: val
 */
ASMFUNC_START(out16)
    movw    %di, %dx
    movq    %rsi, %rax
    outw    %ax, %dx
    ret
ASMFUNC_END(out16)

/*
 * uint32_t in32(uint16_t port);
 * read a 4-byte value from an I/O port specified by port
 * arguments:
 *  %rdi: port
 */
ASMFUNC_START(in32)
    movw    %di, %dx
    inl     %dx, %eax
    ret
ASMFUNC_END(in32)

/*
 * void out32(uint16_t port, uint32_t val);
 * write val to an I/O port specified by port
 * arguments:
 *  %rdi: port
 *  %rsi: val
 */
ASMFUNC_START(out32)
    movw    %di, %dx
    movq    %rsi, %rax
    outl    %eax, %dx
    ret
ASMFUNC_END(out32)

