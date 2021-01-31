#include <Uefi.h>
#include <arch/x64/vmem.h>
#include <common/bootparam.h>
#include <common/util.h>

extern EFI_SYSTEM_TABLE *gst;
#define BS (gst->BootServices)
#define CO (gst->ConOut)

#define GIGA ((uintmax_t)1<<30)
#define MEGA ((uintmax_t)1<<20)
#define KILO ((uintmax_t)1<<10)

#if 0 // for test
static paddr_t vtophys(MyOsX64PageTableEntry *pml4, vaddr_t vaddr)
{
    paddr_t ret;
    MyOsX64PageTableEntry entry;
    int pml4Idx = (vaddr >> 39) & 0x1ffllu;
    int pdptIdx = (vaddr >> 30) & 0x1ffllu;
    int pdIdx = (vaddr >> 21) & 0x1ffllu;
    int ptIdx = (vaddr >> 12) & 0x1ffllu;
    //DEBUG_PRINT("%d %d %d %d\n", pml4Idx, pdptIdx, pdIdx, ptIdx);
    entry = *(pml4 + pml4Idx);
    if (!(entry & PTE_FLAGS_BIT_P)) {
        return (paddr_t)-1; // page not present;
    }
    MyOsX64PageTableEntry *pdpt = (MyOsX64PageTableEntry *)(entry & 0x000ffffffffff000llu);
    //DEBUG_PRINT("0x%016lx 0x%016lx\n", pml4, pdpt);
    entry = *(pdpt + pdptIdx);
    //DEBUG_PRINT("0x%016lx\n", entry);
    if (!(entry & PTE_FLAGS_BIT_P)) {
        return (paddr_t)-1; // page not present;
    }
    if (entry & PDE_FLAGS_BIT_PS) { // 1GiB-page
        ret = (entry & 0x000fffffc0000000llu);
        ret |= (vaddr & 0x000000003fffffffllu);
        return ret;
    }
    MyOsX64PageTableEntry *pd = (MyOsX64PageTableEntry *)(entry & 0x000ffffffffff000llu);
    //DEBUG_PRINT("0x%016lx\n", pd);
    entry = *(pd + pdIdx);
    //DEBUG_PRINT("0x%016lx\n", entry);
    if (!(entry & PTE_FLAGS_BIT_P)) {
        return (paddr_t)-1; // page not present;
    }
    if (entry & PDE_FLAGS_BIT_PS) { // 2MiB-page
        ret = (entry & 0x000fffffffe00000llu);
        ret |= (vaddr & 0x00000000001fffffllu);
        return ret;
    }
    MyOsX64PageTableEntry *pt = (MyOsX64PageTableEntry *)(entry & 0x000ffffffffff000llu);
    //DEBUG_PRINT("0x%016lx\n", pt);
    entry = *(pt + ptIdx);
    //DEBUG_PRINT("0x%016lx\n", entry);
    if (entry & PTE_FLAGS_BIT_P) { // 4KiB-page
        ret = (entry & 0x000ffffffffff000llu);
        ret |= (vaddr & 0x0000000000000fffllu);
        return ret;
    }
    return (paddr_t)-1; // page not present;
}
#endif

EFI_STATUS vmemInit(MyOsBootParameter *bootparam)
{
    EFI_STATUS status;
    MyOsX64PageTableEntry *pml4;
    MyOsX64PageTableEntry *pdptLower, *pdptUpper;
    MyOsX64PageTableEntry *pdKernelSpaceLower, *pdKernelSpaceUpper;

    status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, (EFI_PHYSICAL_ADDRESS *)&pml4);
    if (EFI_ERROR(status)) {
        printf("AllocatePages() error\r\n");
        return EFI_OUT_OF_RESOURCES;
    }
    status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, (EFI_PHYSICAL_ADDRESS *)&pdptLower);
    if (EFI_ERROR(status)) {
        printf("AllocatePages() error\r\n");
        return EFI_OUT_OF_RESOURCES;
    }
    status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, (EFI_PHYSICAL_ADDRESS *)&pdptUpper);
    if (EFI_ERROR(status)) {
        printf("AllocatePages() error\r\n");
        return EFI_OUT_OF_RESOURCES;
    }
    status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, STRAIGHT_MAP_REGION_SIZE_IN_GB, (EFI_PHYSICAL_ADDRESS *)&pdKernelSpaceLower);
    if (EFI_ERROR(status)) {
        printf("AllocatePages() error\r\n");
        return EFI_OUT_OF_RESOURCES;
    }
    status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, STRAIGHT_MAP_REGION_SIZE_IN_GB, (EFI_PHYSICAL_ADDRESS *)&pdKernelSpaceUpper);
    if (EFI_ERROR(status)) {
        printf("AllocatePages() error\r\n");
        return EFI_OUT_OF_RESOURCES;
    }

    memset(pml4, 0, EFI_PAGE_SIZE);
    memset(pdptLower, 0, EFI_PAGE_SIZE);
    memset(pdptUpper, 0, EFI_PAGE_SIZE);
    memset(pdKernelSpaceLower, 0, STRAIGHT_MAP_REGION_SIZE_IN_GB*EFI_PAGE_SIZE);
    memset(pdKernelSpaceUpper, 0, STRAIGHT_MAP_REGION_SIZE_IN_GB*EFI_PAGE_SIZE);

    // straight-map physical address to 0
    paddr_t pbase = 0;
    vaddr_t vbase = 0;
    for (paddr_t i = 0; i < STRAIGHT_MAP_REGION_SIZE_IN_GB; i++) {
        // map as 2MiB-page
        for (int j = 0; j < NUM_PAGE_TABLE_ENTRIES; j++) {
            paddr_t paddr = pbase + i*GIGA + j*2*MEGA;
            vaddr_t vaddr = vbase + i*GIGA + j*2*MEGA;
            pdKernelSpaceLower[i*NUM_PAGE_TABLE_ENTRIES + ((vaddr >> 21) & 0x1ffllu)] = (MyOsX64PageTableEntry)(paddr & 0xfffffffffe00000llu);
            pdKernelSpaceLower[i*NUM_PAGE_TABLE_ENTRIES + ((vaddr >> 21) & 0x1ffllu)] |= PTE_FLAGS_BIT_P | PTE_FLAGS_BIT_RW | PTE_FLAGS_BIT_A | PDE_FLAGS_BIT_PS;
        }
        pdptLower[(((vbase + i*GIGA) >> 30) & 0x1ffllu)] = (MyOsX64PageTableEntry)&pdKernelSpaceLower[i*NUM_PAGE_TABLE_ENTRIES];
        pdptLower[(((vbase + i*GIGA) >> 30) & 0x1ffllu)] |= PTE_FLAGS_BIT_P | PTE_FLAGS_BIT_RW | PTE_FLAGS_BIT_A;
        //DEBUG_PRINT("pdptLower[%d]=0x%016lx\n", (((vbase + i*GIGA) >> 30) & 0x1ffllu), pdptLower[(((vbase + i*GIGA) >> 30) & 0x1ffllu)]);
    }
    pml4[(vbase >> 39) & 0x1ffllu] = (MyOsX64PageTableEntry)pdptLower;
    pml4[(vbase >> 39) & 0x1ffllu] |= PTE_FLAGS_BIT_P | PTE_FLAGS_BIT_RW | PTE_FLAGS_BIT_A;

    // straight-map physical address to KERNEL_VSPACE_OFFSET
    pbase = 0;
    vbase = KERNEL_VSPACE_OFFSET;
    for (paddr_t i = 0; i < STRAIGHT_MAP_REGION_SIZE_IN_GB; i++) {
        // map as 2MiB-page
        for (int j = 0; j < NUM_PAGE_TABLE_ENTRIES; j++) {
            paddr_t paddr = pbase + i*GIGA + j*2*MEGA;
            vaddr_t vaddr = vbase + i*GIGA + j*2*MEGA;
            pdKernelSpaceUpper[i*NUM_PAGE_TABLE_ENTRIES + ((vaddr >> 21) & 0x1ffllu)] = (MyOsX64PageTableEntry)(paddr & 0xfffffffffe00000llu);
            pdKernelSpaceUpper[i*NUM_PAGE_TABLE_ENTRIES + ((vaddr >> 21) & 0x1ffllu)] |= PTE_FLAGS_BIT_P | PTE_FLAGS_BIT_RW | PTE_FLAGS_BIT_A | PDE_FLAGS_BIT_PS;
        }
        pdptUpper[(((vbase + i*GIGA) >> 30) & 0x1ffllu)] = (MyOsX64PageTableEntry)&pdKernelSpaceUpper[i*NUM_PAGE_TABLE_ENTRIES];
        pdptUpper[(((vbase + i*GIGA) >> 30) & 0x1ffllu)] |= PTE_FLAGS_BIT_P | PTE_FLAGS_BIT_RW | PTE_FLAGS_BIT_A;
        //DEBUG_PRINT("pdptLower[%d]=0x%016lx\n", (((vbase + i*GIGA) >> 30) & 0x1ffllu), pdptLower[(((vbase + i*GIGA) >> 30) & 0x1ffllu)]);
    }
    pml4[(vbase >> 39) & 0x1ffllu] = (MyOsX64PageTableEntry)pdptUpper;
    pml4[(vbase >> 39) & 0x1ffllu] |= PTE_FLAGS_BIT_P | PTE_FLAGS_BIT_RW | PTE_FLAGS_BIT_A;

#if 0 // for test
    for (vaddr_t i = 0; i < 5; i++) {
        vaddr_t addr = /*KERNEL_PBASE+*/i*2*MEGA;
        printf("0x%016llx->0x%016llx\n",
                addr,
                vtophys(pml4, addr));
        printf("0x%016llx->0x%016llx\n",
                KERNEL_VSPACE_OFFSET+addr,
                vtophys(pml4, KERNEL_VSPACE_OFFSET+addr));
    }
    for (vaddr_t i = 0; i < 5; i++) {
        vaddr_t addr = /*KERNEL_PBASE+*/STRAIGHT_MAP_REGION_SIZE_IN_GB*GIGA-(i+1)*2*MEGA;
        printf("0x%016llx->0x%016llx\n",
                addr,
                vtophys(pml4, addr));
        printf("0x%016llx->0x%016llx\n",
                KERNEL_VSPACE_OFFSET+addr,
                vtophys(pml4, KERNEL_VSPACE_OFFSET+addr));
    }
#endif

    bootparam->vmemInfo.archInfo = pml4;
    return EFI_SUCCESS;
}

