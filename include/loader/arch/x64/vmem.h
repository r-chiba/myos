#ifndef MYOS_KERNEL_ARCH_X64_VMEM_H_
#define MYOS_KERNEL_ARCH_X64_VMEM_H_

#include <common/types.h>
#include <kernel/config.h>

typedef uint64_t paddr_t;
typedef uint64_t vaddr_t;

#define KERNEL_VSPACE_OFFSET  ((vaddr_t)CONFIG_KERNEL_VSPACE_OFFSET)
#define KERNEL_VBASE ((vaddr_t)CONFIG_KERNEL_VBASE)
#define KERNEL_PBASE ((paddr_t)CONFIG_KERNEL_PBASE)
#define STRAIGHT_MAP_REGION_SIZE_IN_GB ((vaddr_t)CONFIG_STRAIGHT_MAP_REGION_SIZE_IN_GB)

#define V2P(x) ((paddr_t)((vaddr_t)(x) - KERNEL_VSPACE_OFFSET))
#define P2V(x) ((vaddr_t)((paddr_t)(x) + KERNEL_VSPACE_OFFSET))

#define NUM_PAGE_TABLE_ENTRIES 512
typedef uint64_t MyOsX64PageTableEntry;

// See Intel SDM Vol. 3A Sec. 4.5 Table 4-20
#define PTE_FLAGS_BIT_P     ((MyOsX64PageTableEntry)1 << 0) /* Present */
#define PTE_FLAGS_BIT_RW    ((MyOsX64PageTableEntry)1 << 1) /* Read/Write */
#define PTE_FLAGS_BIT_US    ((MyOsX64PageTableEntry)1 << 2) /* User/Supervisor */
#define PTE_FLAGS_BIT_PWT   ((MyOsX64PageTableEntry)1 << 3) /* Page-level write-through */
#define PTE_FLAGS_BIT_PCD   ((MyOsX64PageTableEntry)1 << 4) /* Page-level cache disable */
#define PTE_FLAGS_BIT_A     ((MyOsX64PageTableEntry)1 << 5) /* Accessed */
#define PTE_FLAGS_BIT_D     ((MyOsX64PageTableEntry)1 << 6) /* Dirty */
#define PTE_FLAGS_BIT_PAT   ((MyOsX64PageTableEntry)1 << 7) /* Page Attribute Table */
#define PTE_FLAGS_BIT_G     ((MyOsX64PageTableEntry)1 << 8) /* Global */

// See Intel SDM Vol. 3A Sec. 4.5 Table 4-19
#define PDE_FLAGS_BIT_PS    ((MyOsX64PageTableEntry)1 << 7) /* Page size */

void pageTableInit(void);

#endif // MYOS_KERNEL_ARCH_X64_VMEM_H_
