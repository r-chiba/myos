#ifndef MYOS_KERNEL_ARCH_X64_CONSTANT_H_
#define MYOS_KERNEL_ARCH_X64_CONSTANT_H_

// Control Register bits
#define CR4_FSGSBASE    (1llu << 16)

// Model-Specific Registers
#define MSR_APIC_BASE   0x0000001B
#define MSR_FSBASE      0xC0000100
#define MSR_GSBASE      0xC0000101

// CPUID flags
#define CPUID_FEATURE_EDX_APIC (1u << 9)

#endif // MYOS_KERNEL_ARCH_X64_CONSTANT_H_
