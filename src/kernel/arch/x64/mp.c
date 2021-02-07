#include <arch/x64/mp.h>
#include <arch/x64/pmem.h>
#include <arch/x64/interrupt.h>
#include <arch/x64/asmfunc.h>
#include <arch/x64/asminline.h>
#include <timer.h>
#include <util.h>
#include <vmem.h>

// they are defined in kernel.ld
extern char _trampoline_start[], _trampoline_end[];

// they are defined in trampoline.S
extern uint64_t pml4_trampoline;
extern uint64_t kentry_ap;
extern uint64_t stack_trampoline;

uint64_t nCpus = 0;
uint64_t nAwakedCpus = 0;
MyOsCpuInfo cpuInfo[MAXCPUNUM];

void wakeupAp(uint8_t dest, paddr_t entry)
{
    // set up variables in the trampoline code
    uint64_t *pt =
        (uint64_t *)(entry + ((size_t)&pml4_trampoline-(size_t)_trampoline_start));
    uint64_t *apentry =
        (uint64_t *)(entry + ((size_t)&kentry_ap-(size_t)_trampoline_start));
    uint64_t *stack =
        (uint64_t *)(entry + ((size_t)&stack_trampoline-(size_t)_trampoline_start));

    __asm__ __volatile__ ("movq %%cr3, %%rax" : "=a"(*pt) ::);
    //DEBUG_PRINT("pt: 0x%016lx\n", pt);
    //DEBUG_PRINT("*pt: 0x%016lx\n", *pt);

    *apentry = V2P(kentryForAp);
    //DEBUG_PRINT("apentry: 0x%016lx\n", apentry);
    //DEBUG_PRINT("*apentry: 0x%016lx\n", *apentry);

    *stack = buddyAllocRegion(ZONE_LEGACY, 0) + PAGE_SIZE;
    //DEBUG_PRINT("stack: 0x%016lx\n", stack);
    //DEBUG_PRINT("*stack: 0x%016lx\n", *stack);
    if (!*stack) {
        panic("no available memory for trampline");
    }

    uint64_t cur = nAwakedCpus;
    localApicSendIpi(
        0,
        LAPIC_IPI_DELIVERY_MODE_INIT,
        LAPIC_IPI_LEVEL_ASSERT,
        LAPIC_IPI_TRIGGER_MODE_EDGE,
        LAPIC_IPI_DEST_SHORTHAND_NO,
        dest);
    usleepBusy(10*1000);

    localApicSendIpi(
        entry>>PAGE_SHIFT,
        LAPIC_IPI_DELIVERY_MODE_STARTUP,
        LAPIC_IPI_LEVEL_ASSERT,
        LAPIC_IPI_TRIGGER_MODE_EDGE,
        LAPIC_IPI_DEST_SHORTHAND_NO,
        dest);
    usleepBusy(200);

    localApicSendIpi(
        entry>>PAGE_SHIFT,
        LAPIC_IPI_DELIVERY_MODE_STARTUP,
        LAPIC_IPI_LEVEL_ASSERT,
        LAPIC_IPI_TRIGGER_MODE_EDGE,
        LAPIC_IPI_DEST_SHORTHAND_NO,
        dest);

    for (int i = 0; i < 5000; i++) {
        if (nAwakedCpus > cur) {
            break;
        }
        usleepBusy(1*1000);
    }
    if (nAwakedCpus == cur) {
        panic("can't wake up AP %u\n", dest);
    }
}

void mpInit(void)
{
    DEBUG_PRINT("%s()\n", __func__);

    // nCpus and cpuInfo are initialized in the Local APIC initialization routine
    if (nCpus <= 1) return;

    // copy the trampoline code and data to a page below 1MiB
    paddr_t entry = buddyAllocRegion(ZONE_LEGACY, 0);
    DEBUG_PRINT("trampoline relocated addr: 0x%016lx\n", entry);
    if (!entry) {
        panic("no available memory for trampline");
    }
    memcpy(entry, _trampoline_start, (size_t)(_trampoline_end-_trampoline_start));

    // wake up each AP individually
    for (uint64_t i = 0; i < nCpus; i++) {
        if (getLocalApicId() == cpuInfo[i].localApicId) continue;
        wakeupAp(cpuInfo[i].localApicId, entry);
    }

    buddyFreeRegion(entry, 0);
}
