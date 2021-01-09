#include <arch/x64/asm.h>
#include <arch/x64/constant.h>

    .text

/*
 * save all general-purpose registers to the stack
 * and align the stack to 16-bit boundary
 */
.macro saveGprs
pushq   %rbp
movq    %rsp, %rbp
subq    $0x78, %rsp
movq    %rax, -0x08(%rbp)
movq    %rbx, -0x10(%rbp)
movq    %rcx, -0x18(%rbp)
movq    %rdx, -0x20(%rbp)
movq    %rsi, -0x28(%rbp)
movq    %rdi, -0x30(%rbp)
movq    %r8,  -0x38(%rbp)
movq    %r9,  -0x40(%rbp)
movq    %r10, -0x48(%rbp)
movq    %r11, -0x50(%rbp)
movq    %r12, -0x58(%rbp)
movq    %r13, -0x60(%rbp)
movq    %r14, -0x68(%rbp)
movq    %r15, -0x70(%rbp)
.endm

/*
 * restore the register values saved by saveGprs and the stack
 */
.macro loadGprs
movq    -0x08(%rbp), %rax
movq    -0x10(%rbp), %rbx
movq    -0x18(%rbp), %rcx
movq    -0x20(%rbp), %rdx
movq    -0x28(%rbp), %rsi
movq    -0x30(%rbp), %rdi
movq    -0x38(%rbp), %r8
movq    -0x40(%rbp), %r9
movq    -0x48(%rbp), %r10
movq    -0x50(%rbp), %r11
movq    -0x58(%rbp), %r12
movq    -0x60(%rbp), %r13
movq    -0x68(%rbp), %r14
movq    -0x70(%rbp), %r15
movq    %rbp, %rsp
popq    %rbp
.endm

.macro notifyEoi
movq    $MSR_APIC_BASE, %rcx
rdmsr
shlq    $32, %rdx
orq     %rax, %rdx
andq    $0xfffffffffffff000, %rdx
movl    $0, 0x0b0(%rdx)
.endm

/*
 * void defaultInterruptHandler(void);
 */
ASMFUNC_START(defaultInterruptHandler)
1:
    jmp 1b
ASMFUNC_END(defaultInterruptHandler)

/*
 * void keyboardInterruptHandler(void);
 */
.extern keyboardInterruptHandlerMain;
ASMFUNC_START(keyboardInterruptHandler)
    saveGprs
    call    keyboardInterruptHandlerMain
    notifyEoi
    loadGprs
    iretq
ASMFUNC_END(keyboardInterruptHandler)

