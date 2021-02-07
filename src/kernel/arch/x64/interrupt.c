#include <arch/x64/interrupt.h>
#include <arch/x64/asmfunc.h>
#include <arch/x64/asminline.h>
#include <arch/x64/constant.h>
#include <arch/x64/gdt.h>
#include <arch/x64/mp.h>
#include <util.h>
#include <vmem.h>

static MyOsIdtGateDescriptor idt[NINTDESCS] __attribute__((section(".idt"))) = {0};

static MyOsDescriptorRegister idtr = {
    sizeof(idt)-1,
    (uint64_t)&idt
};

MyOsAcpiMadt *madt = NULL;

extern uint64_t nCpus;
extern MyOsCpuInfo cpuInfo[MAXCPUNUM];

static void setIdtDescriptor(
    MyOsIdtGateDescriptor *desc,
    uint64_t funcAddr,
    uint16_t selector,
    uint8_t p,
    uint8_t dpl,
    uint8_t type,
    uint8_t ist)
{
    memset(desc, 0, sizeof(*desc));
    desc->offsetLowLow  = ((funcAddr >>  0) & 0xffff);
    desc->offsetLowHigh = ((funcAddr >> 16) & 0xffff);
    desc->offsetHigh    = ((funcAddr >> 32) & 0xffffffff);
    desc->segmentSelector = selector;
    desc->attr |= (p & 0x1) << 15;
    desc->attr |= (dpl & 0x3) << 13;
    desc->attr |= (type & 0xf) << 8;
    desc->attr |= (ist & 0x7) << 0;
}

static void setIdtDescriptors(void)
{
    for (int i = 0; i < NINTDESCS; i++) {
        setIdtDescriptor(
            &idt[i],
            (uint64_t)defaultInterruptHandler,
            KERNEL_TEXT_SEGMENT,
            1, 
            0,
            TYPE_INTERRUPT_GATE,
            0);
    }

    setIdtDescriptor(
        &idt[INTVEC_IRQ1],
        (uint64_t)keyboardInterruptHandler,
        KERNEL_TEXT_SEGMENT,
        1, 
        0,
        TYPE_INTERRUPT_GATE,
        0);
}

__attribute__((always_inline))
static void loadIdt(void)
{
    __asm__ __volatile__ (
            "lidt %0\n\t"
            :: "m"(idtr));
}

// disable 8259A PIC
static void disableLegacyPic(void)
{
    // mask all interrupts from secondary PIC
    out8(0xa1, 0xff);
    sfence(); // TODO: need sfence for I/O space?
    // mask all interrupts other than cascade from primary PIC
    out8(0x21, 0xfb);
}

static void findMadt(void)
{
    // find the MADT from the ACPI table
    madt = (MyOsAcpiMadt *)findDescriptionTable(ACPI_SDT_MADT_SIG);
    DEBUG_PRINT("madt address: 0x%016lx\n", (uint64_t)madt);
    if (!madt) {
        panic("madt not found\n");
    }
}

__attribute__((always_inline))
static vaddr_t getLocalApicBaseAddr(void)
{
    uint64_t msrVal = readMsr(MSR_APIC_BASE);
    // base address is at bit 51-12
    return P2V(msrVal & 0x000ffffffffff000ul);
}

__attribute__((always_inline))
int amIBsp(void)
{
    uint64_t msrVal = readMsr(MSR_APIC_BASE);
    return !!(msrVal & (1<<8));
}

static void localApicInit(void)
{
    uint32_t eax, ebx, ecx, edx;
    vaddr_t apicBase;
    volatile uint32_t *svrAddr;
    uint32_t svrVal;

    // check APIC support
    cpuid(0x1, &eax, &ebx, &ecx, &edx);
    KASSERT(edx & CPUID_FEATURE_EDX_APIC, "APIC unsupported\n");

    if (amIBsp()) disableLegacyPic();

    // get the base address of Local APIC register space
    // TODO: must map the register space as UC (cache-disable) memory
    // size of register space is 4 KiB?
#if 0
    uint64_t msrVal = readMsr(MSR_APIC_BASE);
    // base address is at bit 51-12
    apicBase = P2V(msrVal & 0x000ffffffffff000ul);
#endif
    apicBase = getLocalApicBaseAddr();
    DEBUG_PRINT("LOCAL APIC BASE=0x%016lx\n", apicBase);

#if 0
    // enable APIC
    // this operation may not be needed
    msrVal |= (1u << 11);
    writeMsr(MSR_APIC_BASE, msrVal);
#endif

    // set APIC Software Enable/Disable flag
    // (bit 8 of the spurious interrupt vector register)
    svrAddr = (volatile uint32_t *)(apicBase + 0xf0);
    svrVal = *svrAddr;
    svrVal |= (1 << 8);
    *svrAddr = svrVal;

    // find all processors
    if (amIBsp()) {
        uint64_t i = 0;
        while (i < madt->header.length - sizeof(*madt)) {
            MyOsAcpiMadtEntryHeader *header = (MyOsAcpiMadtEntryHeader *)(madt->data + i);
            //printf("type: %u, length: %u\n", header->entryType, header->recordLength);
            if (header->entryType == 0) { // Local APIC
                MyOsAcpiMadtEntryLocalApic *entry =
                    (MyOsAcpiMadtEntryLocalApic *)header;
#if 0
                printf("ACPI Processor ID: %u, APIC ID: %u, Flags: 0x%08x\n",
                        entry->acpiProcessorId,
                        entry->apicId,
                        entry->flags);
#endif
                if ((entry->flags & (1u<<0))
                    || (!(entry->flags & (1u<<0)) && (entry->flags * (1u<<1)))) {
                    // the processor is able to be enabled
                    memset(&cpuInfo[nCpus], 0, sizeof(cpuInfo[nCpus]));
                    cpuInfo[nCpus].acpiProcessorId = entry->acpiProcessorId;
                    cpuInfo[nCpus].localApicId = entry->apicId;
                    nCpus++;
                }
            }
            i += header->recordLength;
        }
        DEBUG_PRINT("available CPUs: %u\n", nCpus);
    }
}

uint8_t getLocalApicId(void)
{
    vaddr_t apicBase = getLocalApicBaseAddr();
    uint32_t idr = *(volatile uint32_t *)(apicBase + LAPIC_IDR_OFFSET);
    return (uint8_t)(idr >> 24);
}

void localApicSendIpi(
        uint8_t vector,
        uint8_t deliveryMode,
        uint8_t level,
        uint8_t triggerMode,
        uint8_t destShorthand,
        uint8_t dest)
{
    uint32_t icrLow = 0, icrHigh = 0;
    vaddr_t apicBase = getLocalApicBaseAddr();

    icrLow |= vector;
    icrLow |= ((deliveryMode & 0x07u) << 8);
    icrLow |= ((level & 0x01u) << 14);
    icrLow |= ((triggerMode & 0x01u) << 15);
    icrLow |= ((destShorthand & 0x03u) << 18);

    icrHigh |= (dest << 24);

    // write to ICR high at first because an IPI is issued when the ICR low is written
    *(volatile uint32_t *)(apicBase + LAPIC_ICR_HIGH_OFFSET) = icrHigh;
    *(volatile uint32_t *)(apicBase + LAPIC_ICR_LOW_OFFSET) = icrLow;

    // wait until the IPI is accepted
    do {
        icrLow = *(volatile uint32_t *)(apicBase + LAPIC_ICR_LOW_OFFSET);
    } while (icrLow & (1u<<12));
}

static uint32_t ioApicReadRegister(uint64_t ioApicBase, uint32_t reg)
{
    *(volatile uint32_t *)(ioApicBase + IOAPIC_IOREGSEL_OFFSET) = reg;
    // TODO: need sfence?
    // I think this can be replaced with softwareMemoryBarrier() when I make sure that
    // I/O APIC space is mapped as UC memory
    sfence(); 
    return *(volatile uint32_t *)(ioApicBase + IOAPIC_IOWIN_OFFSET);
}

static void ioApicWriteRegister(uint64_t ioApicBase, uint32_t reg, uint32_t val)
{
    *(volatile uint32_t *)(ioApicBase + IOAPIC_IOREGSEL_OFFSET) = reg;
    // TODO: need sfence?
    // I think this can be replaced with softwareMemoryBarrier() when I make sure that
    // I/O APIC space is mapped as UC memory
    sfence();
    *(volatile uint32_t *)(ioApicBase + IOAPIC_IOWIN_OFFSET) = val;
}

static void ioApicInit(void)
{
    // find I/O APIC information from the MADT
    // should I consider multiple I/O APIC case?
    MyOsAcpiMadtEntryIoApic *info = NULL;
    uint64_t i = 0;
    while (i < madt->header.length - sizeof(*madt)) {
        MyOsAcpiMadtEntryHeader *header = (MyOsAcpiMadtEntryHeader *)(madt->data + i);
        //printf("type: %u, length: %u\n", header->entryType, header->recordLength);
        if (header->entryType == 1) { // IO APIC
            info = (MyOsAcpiMadtEntryIoApic *)header;
#if 0
            printf("I/O APIC ID: %u, Reserved: %u, I/O APIC Address: 0x%08x, Global System Interrupt Base: %u\n",
                    info->ioApicId,
                    info->reserved,
                    info->ioApicAddress,
                    info->globalSystemInterruptBase);
            break;
#endif
        }
#if 0
        if (header->entryType == 2) { 
            MyOsAcpiMadtEntryInterruptSourceOverride *entry = 
                (MyOsAcpiMadtEntryInterruptSourceOverride *)header;
            printf("bus source: %u, irq source: %u, global system interrupt: 0x%08x, flags: 0x04%x\n",
                    entry->busSource,
                    entry->irqSource,
                    entry->globalSystemInterrupt,
                    entry->flags);
        }
#endif
        i += header->recordLength;
    }
    if (!info) {
        panic("io apic not found\n");
    }
    uint64_t ioApicBase = P2V(info->ioApicAddress);

    uint32_t ver = 0;
    ver = ioApicReadRegister(ioApicBase, 1);

    // redirect the keyboard interrupt to Local APIC IRQ0 of the BSP
    // assume the keyboard interrupt is connected to I/O APIC #1
    ioApicWriteRegister(ioApicBase, IOAPIC_IOREDTBL_LOW_OFFSET(1), 0x00000021); // lower 32-bit
    ioApicWriteRegister(ioApicBase, IOAPIC_IOREDTBL_HIGH_OFFSET(1), 0x00000000); // upper 32-bit
}

static const char keymap[] = {
    /* 00 */ 0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
    /* 10 */ 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0, 'a', 's',
    /* 20 */ 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    /* 30 */ 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
    /* 40 */ 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    /* 50 */ '2', '3', '0', '.'
};

void keyboardInterruptHandlerMain(void)
{
    uint8_t val = 0;
    do {
        val = in8(0x60);
    } while (!(val & 0x80)); // wait until the button is released
    val &= ~0x80;
    if (val < sizeof(keymap)) {
        printf("%c", keymap[val]);
    }
}

void interruptInit(void)
{
    DEBUG_PRINT("%s()\n", __func__);
    disableInterrupts();
    if (amIBsp()) findMadt();
    localApicInit();
    if (amIBsp()) {
        ioApicInit();
        setIdtDescriptors();
    }
    loadIdt();
    enableInterrupts();
}

