#ifndef MYOS_KERNEL_ARCH_X64_ASM_H_
#define MYOS_KERNEL_ARCH_X64_ASM_H_

#define ASMFUNC_START(name) \
    .globl name; \
    .type name,@function; \
    name:
#define ASMFUNC_END(name) \
    .size name, . - name

#endif // MYOS_KERNEL_ARCH_X64_ASM_H_
