#ifndef MYOS_KERNEL_ARCH_X64_VMEM_H_
#define MYOS_KERNEL_ARCH_X64_VMEM_H_

#include <common/types.h>
#include <config.h>
#include <arch/x64/pmem.h>

typedef uint64_t vaddr_t;

#if 0
static_assert(KERNEL_VBASE & ((512ul<<30)-1) == 0,
    "KERNEL_VBASE must be aligned to 512 GiB");
#endif

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
#define PDE_FLAGS_BIT_PS    (1u << 7) /* Page size */

#define PML4IDX_SHIFT 39
#define PDPTIDX_SHIFT 30
#define PDIDX_SHIFT 21
#define PTIDX_SHIFT 12
#define IDX_MASK ((MyOsX64PageTableEntry)0x01ff)
#define PTENTRY_MASK ((MyOsX64PageTableEntry)0x000ffffffffff000)
#define PT_1GB_PAGE_MASK ((MyOsX64PageTableEntry)0x000fffffc0000000)
#define PT_1GB_PAGE_OFFSET ((MyOsX64PageTableEntry)0x000000003fffffff)
#define PT_2MB_PAGE_MASK ((MyOsX64PageTableEntry)0x000fffffffe00000)
#define PT_2MB_PAGE_OFFSET ((MyOsX64PageTableEntry)0x00000000001fffff)
#define PT_4KB_PAGE_MASK ((MyOsX64PageTableEntry)0x000ffffffffff000)
#define PT_4KB_PAGE_OFFSET ((MyOsX64PageTableEntry)0x0000000000000fff)

void pageTableInit(void);

#endif // MYOS_KERNEL_ARCH_X64_VMEM_H_
