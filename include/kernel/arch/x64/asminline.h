#ifndef MYOS_KERNEL_ARCH_X64_ASMINLINE_H_
#define MYOS_KERNEL_ARCH_X64_ASMINLINE_H_

//#include <common/types.h>

__attribute__((always_inline))
static void softwareMemoryBarrier(void)
{
    __asm__ __volatile__("" ::: "memory");
}

__attribute__((always_inline))
static void lfence(void)
{
    __asm__ __volatile__("lfence" ::: "memory");
}

__attribute__((always_inline))
static void sfence(void)
{
    __asm__ __volatile__("sfence" ::: "memory");
}

__attribute__((always_inline))
static void mfence(void)
{
    __asm__ __volatile__("mfence" ::: "memory");
}

#endif // MYOS_KERNEL_ARCH_X64_ASMINLINE_H_
