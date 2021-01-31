#include <arch/x64/vmem.h>
#include <util.h>
#include <arch/x64/asmfunc.h>

paddr_t virtualToPhysical(MyOsX64PageTableEntry *pml4, vaddr_t vaddr)
{
    paddr_t ret;
    MyOsX64PageTableEntry entry;
    int pml4Idx = (vaddr >> PML4IDX_SHIFT) & IDX_MASK;
    int pdptIdx = (vaddr >> PDPTIDX_SHIFT) & IDX_MASK;
    int pdIdx = (vaddr >> PDIDX_SHIFT) & IDX_MASK;
    int ptIdx = (vaddr >> PTIDX_SHIFT) & IDX_MASK;
    //DEBUG_PRINT("%d %d %d %d\n", pml4Idx, pdptIdx, pdIdx, ptIdx);
    entry = *(pml4 + pml4Idx);
    if (!(entry & PTE_FLAGS_BIT_P)) {
        return (paddr_t)-1; // page not present;
    }
    MyOsX64PageTableEntry *pdpt = (MyOsX64PageTableEntry *)(entry & PTENTRY_MASK);
    //DEBUG_PRINT("0x%016lx 0x%016lx\n", pml4, pdpt);
    entry = *(pdpt + pdptIdx);
    //DEBUG_PRINT("0x%016lx\n", entry);
    if (!(entry & PTE_FLAGS_BIT_P)) {
        return (paddr_t)-1; // page not present;
    }
    if (entry & PDE_FLAGS_BIT_PS) { // 1GiB-page
        ret = (entry & PT_1GB_PAGE_MASK);
        ret |= (vaddr & PT_1GB_PAGE_OFFSET);
        return ret;
    }
    MyOsX64PageTableEntry *pd = (MyOsX64PageTableEntry *)(entry & PTENTRY_MASK);
    //DEBUG_PRINT("0x%016lx\n", pd);
    entry = *(pd + pdIdx);
    //DEBUG_PRINT("0x%016lx\n", entry);
    if (!(entry & PTE_FLAGS_BIT_P)) {
        return (paddr_t)-1; // page not present;
    }
    if (entry & PDE_FLAGS_BIT_PS) { // 2MiB-page
        ret = (entry & PT_2MB_PAGE_MASK);
        ret |= (vaddr & PT_2MB_PAGE_OFFSET);
        return ret;
    }
    MyOsX64PageTableEntry *pt = (MyOsX64PageTableEntry *)(entry & PTENTRY_MASK);
    //DEBUG_PRINT("0x%016lx\n", pt);
    entry = *(pt + ptIdx);
    //DEBUG_PRINT("0x%016lx\n", entry);
    if (entry & PTE_FLAGS_BIT_P) { // 4KiB-page
        ret = (entry & PT_4KB_PAGE_MASK);
        ret |= (vaddr & PT_4KB_PAGE_OFFSET);
        return ret;
    }
    return (paddr_t)-1; // page not present;
}

