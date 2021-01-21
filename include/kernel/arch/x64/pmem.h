#ifndef MYOS_KERNEL_ARCH_X64_PMEM_H_
#define MYOS_KERNEL_ARCH_X64_PMEM_H_

#include <common/bootparam.h>
#include <freebsd/queue.h>
#include <util.h>

typedef enum {
  EfiReservedMemoryType,
  EfiLoaderCode,
  EfiLoaderData,
  EfiBootServicesCode,
  EfiBootServicesData,
  EfiRuntimeServicesCode,
  EfiRuntimeServicesData,
  EfiConventionalMemory,
  EfiUnusableMemory,
  EfiACPIReclaimMemory,
  EfiACPIMemoryNVS,
  EfiMemoryMappedIO,
  EfiMemoryMappedIOPortSpace,
  EfiPalCode,
  EfiPersistentMemory,
  EfiMaxMemoryType
} EFI_MEMORY_TYPE;
static_assert(sizeof(EFI_MEMORY_TYPE) == sizeof(uint32_t), "type mismatch");

typedef uintptr_t paddr_t;

// structure used to manage a page-frame
typedef struct __attribute__((packed)) myos_page_frame {
    paddr_t paddr;      // physical address of the page
    uint32_t flags;     // flags for the page
    SLIST_ENTRY(myos_page_frame) next;
} MyOsPageFrame;

typedef struct myos_buddy_free_list {
    SLIST_HEAD(myos_buddy_free_list_head, myos_page_frame) head;
} MyOsBuddyFreeList;

typedef enum {
    ZONE_DMA = 0,   // a zone for memory below 16 MiB (used for legacy-device DMA)
    ZONE_NORMAL,    // a zone for memory not belonging to other zones
    ZONE_NUM
} MyOsMemoryZone;

#define BUDDY_MAX_ORDER (30-PAGE_SHIFT)

void pmemInit(MyOsMemoryMapInfo *mapInfo);
paddr_t buddyAllocRegion(MyOsMemoryZone zone, int order);
void buddyFreeRegion(paddr_t paddr, int order);

#endif // MYOS_KERNEL_ARCH_X64_PMEM_H_
