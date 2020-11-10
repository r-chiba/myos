#include <kernel/util.h>
#include <kernel/physmem.h>
#include <kernel/bootinfo.h>

#if 0
extern char _stext[], etext[];
extern char _sdata[], edata[];
#endif

static void init_pageframe_list(void)
{
    struct page *pages = (struct page *)(uintptr_t)PAGEFRAME_LIST_START_ADDR;
    uint64_t npages = 0;
    uint16_t nentries = *(uint16_t *)(uintptr_t)BOOTINFO_SMAP_NENTRIES_ADDR;
    struct smap_entry *entries = (struct smap_entry *)(uintptr_t)BOOTINFO_SMAP_START_ADDR;
    static_assert(sizeof(struct smap_entry) == BOOTINFO_SMAP_ENTRY_SIZE, "SMAP entry size unmatch");
    for (uint16_t i = 0; i < nentries; i++) {
        if (entries[i].type != SMAP_USABLE) continue;
        uintptr_t start = PAGE_ALIGN(entries[i].start + PAGE_SIZE-1);
        size_t size = entries[i].size - (start - entries[i].start);
        if (size < PAGE_SIZE || size > entries[i].size) {
            continue;
        }
        uint64_t npf = size >> PAGE_SHIFT;
        //DEBUG_PRINT("start: %016lx, size: %lx %lx, npf: %lx\n", start, size, npf*PAGE_SIZE, npf);
        uint64_t n = 0;
        for (uint64_t j = 0; j < npf; j++) {
            struct page *page = pages + npages + n;
            uintptr_t paddr = start + j * PAGE_SIZE;
            if (paddr < PHYSMEM_AVAIL_REGION_START_ADDR) continue;
            page->paddr = paddr;
            page->flags = 0;
            n++;
            if (npages + n >= AVAIL_PAGEFRAME_MAX) break;
        }
        //DEBUG_PRINT("%016lx - %016lx (%lu)\n", start, start+npf*PAGE_SIZE-1);
        npages += n;
    }
    uint64_t *avail_pages = (uint64_t *)(uintptr_t)AVAIL_PAGEFRAME_NUM_ADDR;
    *avail_pages = npages;
    //DEBUG_PRINT("npages=%lu, AVAIL_PAGEFRAME_MAX=%lu\n", npages, AVAIL_PAGEFRAME_MAX);
    //printf("%016lx\n", PAGEFRAME_LIST_START_ADDR+PAGEFRAME_LIST_SIZE);
    //for (uint64_t i = 0; i < 10; i++) {
    //    printf("%p ", pages[i].paddr);
    //}
    //printf("\n");
}

static struct page *get_page_from_paddr(uintptr_t paddr)
{
    uintptr_t paddra = ROUNDDOWN(paddr, PAGE_SIZE);
    uint64_t npages = *(uint64_t *)(uintptr_t)AVAIL_PAGEFRAME_NUM_ADDR;
    struct page *pages = (struct page *)(uintptr_t)PAGEFRAME_LIST_START_ADDR;
    uint64_t l = 0, u = npages-1;
    while (u - l > 1) {
        uint64_t mid = (u + l) / 2;
        uintptr_t addr = pages[mid].paddr;
        //printf("(%lu %lu %lu)", l, u, mid);
        if (addr == paddra) {
            return &pages[mid];
        }
        if (paddra < addr) {
            u = mid;
        } else {
            l = mid + 1;
        }
    }
    return (pages[l].paddr == paddr ? &pages[l] : NULL);
}

// add a memory region to the free list of the order
static void buddy_add_region(uintptr_t addr, size_t size, int order)
{
    struct buddy_free_list *flists =
        (struct buddy_free_list *)(uintptr_t)BUDDY_FREE_LIST_START_ADDR;
    size_t bsize = PAGE_SIZE * (1UL<<order);
    uintptr_t addra = ROUNDDOWN(addr, bsize);
    //DEBUG_PRINT("%lx %lx %d\n", addr, size, order);

    if (order < 0) {
        DEBUG_PRINT("remaining region: [%lx-%lx](%lx)\n", addr, addr+size, size);
        return;
    }

    if (addra != addr || size < bsize) {
        buddy_add_region(addr, size, order-1);
    } else {
        size_t nblocks = size / bsize;
        //DEBUG_PRINT("nblocks=%lu\n", nblocks);
        for (size_t i = 0; i < nblocks; i++) {
            struct page *p = get_page_from_paddr(addr+i*bsize);
            KASSERT(p, "struct page not found. paddr=%p\n", addr+i*bsize);
            SLIST_INSERT_HEAD(&flists[order].head, p, next);
        }
        if (size > bsize * nblocks) {
            buddy_add_region(addr+bsize*nblocks, size-bsize*nblocks, order-1);
        }
    }
}

// prepare a memory region to the free list of the order
// by splitting a region in a higher-order list
static void buddy_prepare_region(int order)
{
    struct buddy_free_list *flists =
        (struct buddy_free_list *)(uintptr_t)BUDDY_FREE_LIST_START_ADDR;
    size_t bsize = PAGE_SIZE * (1UL<<order);

    if (order > BUDDY_MAX_ORDER) {
        return;
    }
    if (!SLIST_EMPTY(&flists[order].head)) {
        // nothing to do
        return;
    }

    // try to prepare a high-order region and split it
    buddy_prepare_region(order+1);
    struct page *p = SLIST_FIRST(&flists[order+1].head);
    if (!p) {
        DEBUG_PRINT("no available memory\n");
        return;
    }
    SLIST_REMOVE(&flists[order+1].head, p, page, next);
    struct page *p2 = get_page_from_paddr(p->paddr + bsize);
    SLIST_INSERT_HEAD(&flists[order].head, p, next);
    SLIST_INSERT_HEAD(&flists[order].head, p2, next);
}

static uintptr_t buddy_alloc_region(int order)
{
    struct buddy_free_list *flists =
        (struct buddy_free_list *)(uintptr_t)BUDDY_FREE_LIST_START_ADDR;

    if (order > BUDDY_MAX_ORDER) return (uintptr_t)NULL;
    buddy_prepare_region(order);
    struct page *p = SLIST_FIRST(&flists[order].head);
    if (!p) return (uintptr_t)NULL;
    SLIST_REMOVE(&flists[order].head, p, page, next);
    return p->paddr;
}

static void buddy_free_region(uintptr_t paddr, size_t size)
{
    if ((paddr % PAGE_SIZE != 0)
        || (size % PAGE_SIZE != 0)
        || (size > PAGE_SIZE * (1UL << BUDDY_MAX_ORDER))) {
        DEBUG_PRINT("invalid argument. paddr=%p, size=%lx\n", paddr, size);
        return;
    }
    struct page *p = get_page_from_paddr(paddr);
    int order = log2(size >> PAGE_SHIFT);
    KASSERT((0 <= order && order <= BUDDY_MAX_ORDER),
            "invalid order. order=%d, size=%lx\n", order, size);
    struct buddy_free_list *flists =
        (struct buddy_free_list *)(uintptr_t)BUDDY_FREE_LIST_START_ADDR;
    KASSERT(p, "struct page not found. paddr=%p\n", paddr);

    // search for a buddy region
    struct page *buddy = NULL, *pp;
    size_t ubsize = PAGE_SIZE * (1UL<<(order+1));
    uintptr_t bpaddr = (paddr % ubsize == 0 ? paddr + size : paddr - size);
    SLIST_FOREACH(pp, &flists[order].head, next) {
        if (pp->paddr == bpaddr) {
            buddy = pp;
            break;
        }
    }
    if (buddy != NULL) {
        // merge the regions and add them to the high-order list
        SLIST_REMOVE(&flists[order].head, buddy, page, next);
        SLIST_INSERT_HEAD(&flists[order+1].head,
                            (paddr % ubsize == 0 ? p : buddy), next);
    } else {
        SLIST_INSERT_HEAD(&flists[order].head, p, next);
    }
}

static void init_buddy_system(void)
{
    struct buddy_free_list *flists =
        (struct buddy_free_list *)(uintptr_t)BUDDY_FREE_LIST_START_ADDR;
    uint16_t nentries = *(uint16_t *)(uintptr_t)BOOTINFO_SMAP_NENTRIES_ADDR;
    struct smap_entry *entries = (struct smap_entry *)(uintptr_t)BOOTINFO_SMAP_START_ADDR;

    for (int i = 0; i <= BUDDY_MAX_ORDER; i++) {
        SLIST_INIT(&flists[i].head);
    }

    for (uint16_t i = 0; i < nentries; i++) {
        if (entries[i].type != SMAP_USABLE) continue;
        uintptr_t start = PAGE_ALIGN(entries[i].start + PAGE_SIZE-1);
        size_t size = entries[i].size - (start - entries[i].start);
        if (start < PHYSMEM_AVAIL_REGION_START_ADDR) {
            if (start + size > PHYSMEM_AVAIL_REGION_START_ADDR) {
                size -= PHYSMEM_AVAIL_REGION_START_ADDR - start;
                start = PHYSMEM_AVAIL_REGION_START_ADDR;
            } else {
                continue;
            }
        }
        buddy_add_region(start, size, BUDDY_MAX_ORDER);
    }

#if 0
    for (int i = BUDDY_MAX_ORDER; i >= 0; i--) {
        struct page *p;
        if (i == BUDDY_MAX_ORDER || i == 1) {
            DEBUG_PRINT("order:%d\n", i);
            SLIST_FOREACH(p, &flists[i].head, next) {
                printf("%p ", p->paddr);
            }
            printf("\n");
        }
    }
#endif
}

static void print_system_memory_map(void)
{
    uint16_t nentries = *(uint16_t *)BOOTINFO_SMAP_NENTRIES_ADDR;
    uint8_t *smap = (uint8_t *)BOOTINFO_SMAP_START_ADDR;
    printf("physical memory map (e820):\n");
    for (int i = 0; i < nentries; i++) {
        uint8_t *entry = smap + i*BOOTINFO_SMAP_ENTRY_SIZE;
        uint64_t base = *(uint64_t *)entry;
        uint64_t size = *(uint64_t *)(entry + sizeof(uint64_t));
        uint32_t type = *(uint32_t *)(entry + sizeof(uint64_t)*2);
        printf("[%016lx-%016lx] %s(%u)\n", base, base+size-1, (type == 1 ? "usable" : "reserved"), type);
    }
}

void init_physmem(void)
{
#if 0
    size_t text_size = (size_t)((uintptr_t)(&etext) - (uintptr_t)(&stext));
    size_t data_size = (size_t)((uintptr_t)(&edata) - (uintptr_t)(&sdata));
#endif
    memset((void *)(uintptr_t)PHYSMEM_MANAGEMENT_REGION_START_ADDR,
            0, PHYSMEM_MANAGEMENT_REGION_SIZE);
    print_system_memory_map();
    init_pageframe_list();
    init_buddy_system();
    DEBUG_PRINT("physmem init done!\n");

    // buddy system test
    uintptr_t addr = buddy_alloc_region(0);
    DEBUG_PRINT("%p\n", addr);
}

