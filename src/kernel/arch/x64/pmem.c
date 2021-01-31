#include <arch/x64/pmem.h>
#include <vmem.h>
#include <status.h>

extern char _kernel_start[], _kernel_end[];

MyOsPageFrame *pages; // list of all available page-frames
uint64_t nAvailPages;
MyOsBuddyFreeList freeLists[ZONE_NUM][BUDDY_MAX_ORDER+1];

static int isUsableMemoryRegion(const MyOsMemoryMapEntry *entry)
{
    switch (entry->type) {
    case EfiBootServicesCode:
    case EfiBootServicesData:
    case EfiConventionalMemory:
        return 1;
    default:
        return 0;
    }
}

static MyOsMemoryZone getZone(paddr_t start, paddr_t end, paddr_t *border)
{
#define ZONE_DMA_BORDER (16lu << 20)
    if (end - 1< ZONE_DMA_BORDER) {
        return ZONE_DMA;
    } else if (start >= ZONE_DMA_BORDER) {
        return ZONE_NORMAL;
    }
    if (border) *border = ZONE_DMA_BORDER;
    return ZONE_NUM;
}

static MYOS_STATUS pageInit(const MyOsMemoryMapInfo *info)
{
    MYOS_STATUS status = MYOS_OK;
    uint8_t *map = (uint8_t *)info->mapBase;
    uint64_t nEntries = info->mapSize / info->entrySize;

    // calculate available memory size
    uint64_t availPages = 0;
    for (uint64_t i = 0; i < nEntries; i++) {
        MyOsMemoryMapEntry *entry = (MyOsMemoryMapEntry *)(map + i * info->entrySize);
#if 0
        //DEBUG_PRINT("%08x %016lx %016lx %016lx %016lx\r\n",
        //        entry->type,
        //        entry->physStart,
        //        entry->virtStart,
        //        entry->nPages,
        //        entry->attr);
        if (entry->physStart+entry->nPages*PAGE_SIZE-1 < 16<<20
            //&& isUsableMemoryRegion(entry)
            ) {
            DEBUG_PRINT("%08x %016lx-%016lx %016lx\r\n",
                    entry->type,
                    entry->physStart,
                    entry->physStart+entry->nPages*PAGE_SIZE-1,
                    entry->attr);
        }
#endif
        if (isUsableMemoryRegion(entry)) {
            availPages += entry->nPages;
        }
    }

    // allocate a region used to manage available memory
    uint64_t mgmtPages =
        ROUNDUP(availPages * sizeof(MyOsPageFrame), PAGE_SIZE) >> PAGE_SHIFT;
    for (uint64_t i = 0; i < nEntries; i++) {
        MyOsMemoryMapEntry *entry = (MyOsMemoryMapEntry *)(map + i * info->entrySize);
        if (isUsableMemoryRegion(entry) && mgmtPages <= entry->nPages) {
            entry->nPages -= mgmtPages;
            pages = (MyOsPageFrame *)P2V(entry->physStart + entry->nPages*PAGE_SIZE);
            break;
        }
    }
    nAvailPages = availPages - mgmtPages;
    DEBUG_PRINT("avail memory = %llu (MiB) / %llu (pages)\n", nAvailPages>>(20-PAGE_SHIFT), nAvailPages);
    DEBUG_PRINT("mgmt area: %p-%p\n", pages, (paddr_t)pages+mgmtPages*PAGE_SIZE);

    // initialize the page-frame list
    uint64_t cur = 0;
    for (uint64_t i = 0; i < nEntries; i++) {
        MyOsMemoryMapEntry *entry = (MyOsMemoryMapEntry *)(map + i * info->entrySize);
        if (!isUsableMemoryRegion(entry)) continue;
        for (uint64_t j = 0; j < entry->nPages; j++) {
            MyOsPageFrame *page = pages + cur + j;
            page->paddr = entry->physStart + j*PAGE_SIZE;
            page->flags = 0;
        }
        cur += entry->nPages;
    }
    return status;
}

static MyOsPageFrame *getPageFrameFromAddr(paddr_t paddr)
{
    paddr_t paddra = ROUNDDOWN(paddr, PAGE_SIZE);
    uint64_t l = 0, u = nAvailPages;
    while (u - l > 1) {
        uint64_t mid = (u + l) / 2;
        paddr_t addr = pages[mid].paddr;
        //DEBUG_PRINT("%p (%lu %lu) %lu:%p ", paddra, l, u, mid, addr);
        if (addr == paddra) {
            return &pages[mid];
        }
        if (paddra < addr) {
            u = mid;
        } else {
            l = mid + 1;
        }
    }
    return (pages[l].paddr == paddra ? &pages[l] : NULL);
}

// add a memory region to the free list of the order
// assume [start, end) is in one memory zone
static void buddyAddRegion(paddr_t start, paddr_t end, MyOsMemoryZone zone, int order)
{
    if ((zone < ZONE_DMA || zone >= ZONE_NUM)
        || (order < 0 || order > BUDDY_MAX_ORDER)) {
        DEBUG_PRINT("[%lx-%lx], %d, %d\n", start, end, zone, order);
        return;
    }

    MyOsBuddyFreeList *fl = &freeLists[zone][order];
    size_t size = end - start;
    size_t bsize = PAGE_SIZE << order;
    paddr_t starta = ROUNDDOWN(start, bsize);

    if (starta != start || size < bsize) {
        buddyAddRegion(start, end, zone, order-1);
    } else {
        size_t nblocks = size / bsize;
        for (size_t i = 0; i < nblocks; i++) {
            MyOsPageFrame *p = getPageFrameFromAddr(start+i*bsize);
            KASSERT(p, "struct page not found. paddr=%p\n", start+i*bsize);
            SLIST_INSERT_HEAD(&fl->head, p, next);
        }
        if (size > bsize * nblocks) {
            buddyAddRegion(start+bsize*nblocks, end, zone, order);
        }
    }
}

// prepare a memory region to the free list of the order
// by splitting a region in a higher-order list
static void buddyPrepareRegion(MyOsMemoryZone zone, int order)
{
    if ((zone < ZONE_DMA || zone >= ZONE_NUM)
        || (order < 0 || order > BUDDY_MAX_ORDER)) {
        DEBUG_PRINT("%d, %d\n", zone, order);
        return;
    }

    MyOsBuddyFreeList *fl = &freeLists[zone][order];
    if (order == BUDDY_MAX_ORDER || !SLIST_EMPTY(&fl->head)) {
        // nothing to do
        return;
    }

    // try to prepare a high-order region and split it
    buddyPrepareRegion(zone, order+1);
    MyOsBuddyFreeList *flHigh = &freeLists[zone][order+1];
    MyOsPageFrame *p = SLIST_FIRST(&flHigh->head);
    if (!p) {
        DEBUG_PRINT("no available memory\n");
        return;
    }
    SLIST_REMOVE(&flHigh->head, p, myos_page_frame, next);
    MyOsPageFrame *p2 = getPageFrameFromAddr(p->paddr + (PAGE_SIZE << order));
    SLIST_INSERT_HEAD(&fl->head, p, next);
    SLIST_INSERT_HEAD(&fl->head, p2, next);
}

paddr_t buddyAllocRegion(MyOsMemoryZone zone, int order)
{
    if (order > BUDDY_MAX_ORDER) return (paddr_t)NULL;
    buddyPrepareRegion(zone, order);
    MyOsBuddyFreeList *fl = &freeLists[zone][order];
    MyOsPageFrame *p = SLIST_FIRST(&fl->head);
    if (!p) return (paddr_t)NULL;
    SLIST_REMOVE(&fl->head, p, myos_page_frame, next);
    return p->paddr;
}

void buddyFreeRegion(paddr_t paddr, int order)
{
    //DEBUG_PRINT("%p %d\n", paddr, order);
    if (order < 0 || order > BUDDY_MAX_ORDER) {
        DEBUG_PRINT("invalid order: %d\n", order);
        return;
    }
    size_t bsize = PAGE_SIZE << order;
    if (paddr != ROUNDDOWN(paddr, bsize)) {
        DEBUG_PRINT("invalid addr: %p\n", paddr);
        return;
    }
    MyOsPageFrame *p = getPageFrameFromAddr(paddr);
    MyOsMemoryZone zone = getZone(paddr, paddr+bsize, NULL);
    if (!p || zone == ZONE_NUM) {
        DEBUG_PRINT("invalid arguments. paddr=0x%lx, zone=%d, order=%d\n",
            paddr, zone, order);
        return;
    }
    MyOsBuddyFreeList *fl = &freeLists[zone][order];

    // search for a buddy region
    MyOsPageFrame *buddy = NULL;
    if (order < BUDDY_MAX_ORDER) {
        size_t ubsize = bsize << 1;
        uintptr_t bpaddr = (paddr % ubsize == 0 ? paddr + bsize : paddr - bsize);
        MyOsPageFrame *pp;
        // TODO: reduce complexity from O(n)
        SLIST_FOREACH(pp, &fl->head, next) {
            if (pp->paddr == bpaddr) {
                buddy = pp;
                break;
            }
        }
    }
    //DEBUG_PRINT("%p %d %p\n", paddr, order, buddy);
    if (buddy != NULL) {
        // merge the regions and add them to the high-order list
        SLIST_REMOVE(&fl->head, buddy, myos_page_frame, next);
        buddyFreeRegion((paddr & ~((bsize << 1) - 1)), order+1);
    } else {
        SLIST_INSERT_HEAD(&fl->head, p, next);
    }
}

static void buddyInit(const MyOsMemoryMapInfo *info)
{
    for (int i = 0; i < ZONE_NUM; i++) {
        for (int j = 0; j <= BUDDY_MAX_ORDER; j++) {
            SLIST_INIT(&freeLists[i][j].head);
        }
    }

    uint8_t *map = (uint8_t *)info->mapBase;
    uint64_t nEntries = info->mapSize / info->entrySize;
    for (uint64_t i = 0; i < nEntries; i++) {
        MyOsMemoryMapEntry *entry = (MyOsMemoryMapEntry *)(map + i * info->entrySize);
        if (!isUsableMemoryRegion(entry)) continue;
        paddr_t start = entry->physStart;
        paddr_t end = entry->physStart + entry->nPages * PAGE_SIZE;
        while (1) {
            paddr_t border;
            MyOsMemoryZone zone = getZone(start, end, &border);
            if (zone != ZONE_NUM) {
                buddyAddRegion(start, end, zone, BUDDY_MAX_ORDER);
                break;
            } else {
                MyOsMemoryZone zone = getZone(start, border, NULL);
                buddyAddRegion(start, border, zone, BUDDY_MAX_ORDER);
                start = border;
            }
        }
    }
#if 1
    // confirm all available memory is managed by buddy-system
    uint64_t avail = 0;
    for (MyOsMemoryZone zone = ZONE_DMA; zone < ZONE_NUM; zone++) {
        for (int order = 0; order <= BUDDY_MAX_ORDER; order++) {
            uint64_t n = 0;
            MyOsPageFrame *p;
            SLIST_FOREACH(p, &freeLists[zone][order].head, next) {
                n++;
                avail += PAGE_SIZE << order;
            }
        }
    }
    KASSERT(avail>>PAGE_SHIFT == nAvailPages,
            "available memory inconsistent %llu:%llu\n",
            avail>>PAGE_SHIFT, nAvailPages);
    //DEBUG_PRINT("avail memory = %llu (MiB) / %llu (pages)\n", avail >> 20, avail >> PAGE_SHIFT);
#endif
}

static void buddyTest(void)
{
    paddr_t buf = buddyAllocRegion(ZONE_NORMAL, 11);
    paddr_t buf2 = buddyAllocRegion(ZONE_NORMAL, 11);
    DEBUG_PRINT("buf=%p, buf2=%p\n", buf, buf2);
    for (MyOsMemoryZone zone = ZONE_DMA; zone < ZONE_NUM; zone++) {
        for (int order = 11; order <= BUDDY_MAX_ORDER; order++) {
            uint64_t n = 0;
            MyOsPageFrame *p;
            SLIST_FOREACH(p, &freeLists[zone][order].head, next) {
                n++;
            }
            if (zone == ZONE_NORMAL) DEBUG_PRINT("%d %d: %lu\n", zone, order, n);
        }
    }
    if (buf) buddyFreeRegion(buf, 11);
    if (buf2) buddyFreeRegion(buf2, 11);
    for (MyOsMemoryZone zone = ZONE_DMA; zone < ZONE_NUM; zone++) {
        for (int order = 11; order <= BUDDY_MAX_ORDER; order++) {
            uint64_t n = 0;
            MyOsPageFrame *p;
            SLIST_FOREACH(p, &freeLists[zone][order].head, next) {
                n++;
            }
            if (zone == ZONE_NORMAL) DEBUG_PRINT("%d %d: %lu\n", zone, order, n);
        }
    }
}

void pmemInit(MyOsMemoryMapInfo *mapInfo)
{
    DEBUG_PRINT("%s()\n", __func__);
    pageInit(mapInfo);
    buddyInit(mapInfo);

#if 0
    buddyTest();
#endif
}

