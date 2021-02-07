#ifndef MYOS_KERNEL_ARCH_X64_PCPU_H_
#define MYOS_KERNEL_ARCH_X64_PCPU_H_

#include <common/types.h>
#include <arch/x64/mp.h>

typedef struct __attribute__((packed)) myos_per_cpu_data {
    uint64_t id;
} MyOsPerCpuData;

extern MyOsPerCpuData pcpu[MAXCPUNUM];

#endif // MYOS_KERNEL_ARCH_X64_PCPU_H_
