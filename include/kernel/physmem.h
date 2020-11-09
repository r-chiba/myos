#ifndef MYOS_KERNEL_PHYSMEM_H_
#define MYOS_KERNEL_PHYSMEM_H_

#include <kernel/types.h>
#include <freebsd/sys/sys/queue.h>

// page-frame definitions
#define PAGE_SHIFT 21
#define PAGE_SIZE (1UL<<PAGE_SHIFT)
#define PAGE_MASK (PAGE_SIZE-1)
#define PAGE_OFFSET(addr) ((addr) & PAGE_MASK)
#define PAGE_ALIGN(addr) ((addr) & ~PAGE_MASK)

#define AVAIL_PHYS_MEM_MAX (128*(1UL<<30))
#define AVAIL_PAGEFRAME_MAX (AVAIL_PHYS_MEM_MAX >> PAGE_SHIFT)

// structure used to manage a page-frame
struct page {
    uintptr_t paddr;    // address of the page
    uint32_t flags;     // flags for the page
    SLIST_ENTRY(page) next;
} __attribute__((packed));

struct buddy_free_list {
    SLIST_HEAD(buddy_free_list_head, page) head;
};

// kernel physical memory layout
#define PHYSMEM_MANAGEMENT_REGION_START_ADDR (1UL<<20)
#define PHYSMEM_MANAGEMENT_REGION_SIZE (3*(1UL<<20))
#define PHYSMEM_AVAIL_REGION_START_ADDR (PHYSMEM_MANAGEMENT_REGION_START_ADDR + PHYSMEM_MANAGEMENT_REGION_SIZE)
#define AVAIL_PAGEFRAME_NUM_ADDR (PHYSMEM_MANAGEMENT_REGION_START_ADDR + 0)
#define PAGEFRAME_LIST_START_ADDR (PHYSMEM_MANAGEMENT_REGION_START_ADDR + 64)
#define PAGEFRAME_LIST_SIZE (AVAIL_PAGEFRAME_MAX*sizeof(struct page))
#define BUDDY_FREE_LIST_START_ADDR (PHYSMEM_MANAGEMENT_REGION_START_ADDR + PAGEFRAME_LIST_SIZE)
#define BUDDY_FREE_LIST_SIZE
static_assert(PAGEFRAME_LIST_SIZE+64 <= PHYSMEM_MANAGEMENT_REGION_SIZE, "physmem management area overflowed");

// physical memory zone definitions
#define ZONE_DMA    0 // a zone for memory below 2^24 B = 16 MiB (used for legacy-device DMA)
#define ZONE_32BIT  1 // a zone for memory below 2^32 B = 4 GiB
#define ZONE_NORMAL 2 // a zone for memory not belonging to other zones

// page flag definitions
#define PAGE_LOCKED
#define PAGE_DIRTY

#define BUDDY_MAX_ORDER 9

#define SMAP_USABLE     1
#define SMAP_RESERVED   2

// system map entry retrieved from BIOS
struct smap_entry {
    uint64_t start;
    uint64_t size;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));

uintmax_t get_max_pageframes(uint32_t type);
void init_physmem(void);

#endif  // MYOS_KERNEL_PHYSMEM_H_
