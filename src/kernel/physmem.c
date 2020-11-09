#include <kernel/util.h>
#include <kernel/physmem.h>
#include <kernel/bootinfo.h>

struct page *pmem_map;
#if 0
struct buddy_free_area free_area[BUDDY_MAX_ORDER];

extern char _stext[], etext[];
extern char _sdata[], edata[];
#endif

uintmax_t get_max_pageframes(uint32_t type)
{
    uintmax_t ret = 0;
    uint16_t nentries = *(uint16_t *)BOOTINFO_SMAP_NENTRIES_ADDR;
    struct smap_entry *entries = (struct smap_entry *)BOOTINFO_SMAP_START_ADDR;
    static_assert(sizeof(struct smap_entry) == BOOTINFO_SMAP_ENTRY_SIZE, "SMAP entry size unmatch");
    printf("nentries=%lu\n", nentries);
    for (uint16_t i = 0; i < nentries; i++) {
        printf("%016lx %016lx %u\n", entries[i].start, entries[i].size, entries[i].type);
        if (entries[i].type != type) continue;
        uintmax_t epf = (entries[i].start + entries[i].size) >> PAGE_SHIFT;
        printf("epf=%lu\n", epf);
        if (ret < epf) ret = epf;
    }
    return ret;
}

static void init_pageframe_list(void)
{
    struct page *pages = (struct page *)(uintptr_t)PAGEFRAME_LIST_START_ADDR;
    uintmax_t npages = 0;
    uint16_t nentries = *(uint16_t *)(uintptr_t)BOOTINFO_SMAP_NENTRIES_ADDR;
    struct smap_entry *entries = (struct smap_entry *)(uintptr_t)BOOTINFO_SMAP_START_ADDR;
    static_assert(sizeof(struct smap_entry) == BOOTINFO_SMAP_ENTRY_SIZE, "SMAP entry size unmatch");
    for (uint16_t i = 0; i < nentries; i++) {
        if (entries[i].type != SMAP_USABLE) continue;
        uintptr_t start = PAGE_ALIGN(entries[i].start + PAGE_SIZE-1);
        uintptr_t size = entries[i].size - (start - entries[i].start);
        if ((size < PAGE_SIZE)
            || (size > entries[i].size)) {
            continue;
        }
        uintmax_t npf = size >> PAGE_SHIFT;
        printf("start: %016lx, size: %lx %lx, npf: %lx\n", start, size, npf*PAGE_SIZE, npf);
        uintmax_t n = 0;
        for (uintmax_t j = 0; j < npf; j++) {
            struct page *page = pages + npages + n;
            uintptr_t paddr = start + j * PAGE_SIZE;
            if (paddr < PHYSMEM_AVAIL_REGION_START_ADDR) continue;
            page->paddr = paddr;
            page->flags = 0;
            n++;
            if (npages + n >= AVAIL_PAGEFRAME_MAX) break;
        }
        printf("%016lx - %016lx (%lu)\n", start, start+npf*PAGE_SIZE-1);
        npages += n;
    }
    uintmax_t *avail_pages = (uintmax_t *)(uintptr_t)AVAIL_PAGEFRAME_NUM_ADDR;
    *avail_pages = npages;
    printf("npages=%lu, AVAIL_PAGEFRAME_MAX=%lu\n", npages, AVAIL_PAGEFRAME_MAX);
    //printf("%016lx\n", PAGEFRAME_LIST_START_ADDR+PAGEFRAME_LIST_SIZE);
    //for (uintmax_t i = 0; i < 10; i++) {
    //    printf("%p ", pages[i].paddr);
    //}
    //printf("\n");
}

static struct page *get_page_from_paddr(uintptr_t paddr)
{
    uintptr_t paddra = ROUNDDOWN(paddr, PAGE_SIZE);
    uintmax_t npages = *(uintmax_t *)(uintptr_t)AVAIL_PAGEFRAME_NUM_ADDR;
    struct page *pages = (struct page *)(uintptr_t)PAGEFRAME_LIST_START_ADDR;
    uintmax_t l = 0, u = npages-1;
    while (u - l > 1) {
        uintmax_t mid = (u + l) / 2;
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

static int buddy_add_region(uintptr_t start, size_t size, int order)
{
    int ret = 0;
    struct buddy_free_list *flists =
        (struct buddy_free_list *)(uintptr_t)BUDDY_FREE_LIST_START_ADDR;
    size_t bsize = PAGE_SIZE * (1UL<<order);
    uintptr_t starta = ROUNDDOWN(start, bsize);
    //printf("(%lx(%lx) %lx(%lx) %d)", start, starta, size, bsize, order);

    if (order < 0) {
        //printf("(%lx(%lx) %lx(%lx) %d)\n", start, starta, size, bsize, order);
        return -1;
    }

    if (starta != start || size < bsize) {
        return buddy_add_region(start, size, order-1);
    } else {
        struct buddy_free_list flist = flists[order];
        size_t nblock = size / bsize;
        printf("%lu\n", nblock);
        for (size_t i = 0; i < nblock; i++) {
            struct page * p = get_page_from_paddr(start+i*bsize);
            KASSERT(p, "struct page not found. paddr=%p\n", start+i*bsize);
            //printf("%p, %lu ", p, i);
            SLIST_INSERT_HEAD(&flist.head, p, next);
        }
        if (size > bsize * nblock) {
            ret = buddy_add_region(start+bsize*nblock, size-bsize*nblock, order-1);
        }
        return ret;
    }
}

static void init_buddy_system(void)
{
    struct buddy_free_list *flists =
        (struct buddy_free_list *)(uintptr_t)BUDDY_FREE_LIST_START_ADDR;
    uint16_t nentries = *(uint16_t *)(uintptr_t)BOOTINFO_SMAP_NENTRIES_ADDR;
    struct smap_entry *entries = (struct smap_entry *)(uintptr_t)BOOTINFO_SMAP_START_ADDR;

    for (int i = 0; i < BUDDY_MAX_ORDER; i++) {
        struct buddy_free_list flist = flists[i];
        SLIST_INIT(&flist.head);
    }

    for (uint16_t i = 0; i < nentries; i++) {
        if (entries[i].type != SMAP_USABLE) continue;
        uintptr_t start = PAGE_ALIGN(entries[i].start + PAGE_SIZE-1);
        uintptr_t size = entries[i].size - (start - entries[i].start);
        if (start < PHYSMEM_AVAIL_REGION_START_ADDR) {
            if (start + size > PHYSMEM_AVAIL_REGION_START_ADDR) {
                size -= PHYSMEM_AVAIL_REGION_START_ADDR - start;
                start = PHYSMEM_AVAIL_REGION_START_ADDR;
            } else {
                continue;
            }
        }
        int ret = buddy_add_region(start, size, BUDDY_MAX_ORDER);
        printf("ret=%d\n", ret);
    }
}

void init_physmem(void)
{
#if 0
    size_t text_size = (size_t)((uintptr_t)(&etext) - (uintptr_t)(&stext));
    size_t data_size = (size_t)((uintptr_t)(&edata) - (uintptr_t)(&sdata));
#endif
    memset((void *)(uintptr_t)PHYSMEM_MANAGEMENT_REGION_START_ADDR, 0, PHYSMEM_MANAGEMENT_REGION_SIZE);
    init_pageframe_list();
    init_buddy_system();
    printf("physmem init done!\n");
}

