#ifndef MYOS_KERNEL_ARCH_X64_DESCRIPTOR_H_
#define MYOS_KERNEL_ARCH_X64_DESCRIPTOR_H_

#include <common/types.h>

// used for GDT and IDT
typedef struct __attribute__((packed)) myos_descriptor_register {
    uint16_t descriptorTableLimit;
    uint64_t descriptorTableAddress;
} MyOsDescriptorRegister;

#endif // MYOS_KERNEL_ARCH_X64_DESCRIPTOR_H_
