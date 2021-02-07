#ifndef MYOS_KERNEL_ARCH_X64_MP_H_
#define MYOS_KERNEL_ARCH_X64_MP_H_

#include <common/types.h>

#define MAXCPUNUM 256

typedef struct myos_cpu_info {
    uint8_t acpiProcessorId;
    uint8_t localApicId;
} MyOsCpuInfo;

#endif // MYOS_KERNEL_ARCH_X64_MP_H_
