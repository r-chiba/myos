#ifndef MYOS_KERNEL_ARCH_X64_ASMFUNC_H_
#define MYOS_KERNEL_ARCH_X64_ASMFUNC_H_

#include <common/types.h>
#include <common/bootparam.h>

void kentry(MyOsBootParameter *bootparam);
void kentryForAp(void);
void halt(void);
void disableInterrupts(void);
void enableInterrupts(void);
uint64_t readMsr(uint32_t addr);
void writeMsr(uint32_t addr, uint64_t val);
void cpuid(
         uint32_t func,
         uint32_t *eax,
         uint32_t *ebx,
         uint32_t *ecx,
         uint32_t *edx);
uint8_t in8(uint16_t port);
void out8(uint16_t port, uint8_t val);
uint16_t in16(uint16_t port);
void out16(uint16_t port, uint16_t val);
uint32_t in32(uint16_t port);
void out32(uint16_t port, uint32_t val);

#endif // MYOS_KERNEL_ARCH_X64_ASMFUNC_H_
