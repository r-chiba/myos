#ifndef MYOS_KERNEL_VMEM_H_
#define MYOS_KERNEL_VMEM_H_

#include <common/types.h>
#include <config.h>
#include <arch/x64/vmem.h>

#define KERNEL_VSPACE_OFFSET  ((vaddr_t)CONFIG_KERNEL_VSPACE_OFFSET)
#define KERNEL_VBASE ((vaddr_t)CONFIG_KERNEL_VBASE)
#define KERNEL_PBASE ((paddr_t)CONFIG_KERNEL_PBASE)
#define V2P(x) ((paddr_t)((vaddr_t)(x) - KERNEL_VSPACE_OFFSET))
#define P2V(x) ((vaddr_t)((paddr_t)(x) + KERNEL_VSPACE_OFFSET))

static_assert((KERNEL_VSPACE_OFFSET & (((vaddr_t)512<<30)-1)) == 0,
    "KERNEL_VSPACE_OFFSET must be aligned to 512 GiB");

#endif // MYOS_KERNEL_VMEM_H_
