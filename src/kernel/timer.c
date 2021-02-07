#include <timer.h>
#include <vmem.h>
#include <util.h>

vaddr_t hpetBase = 0;

__attribute__((always_inline))
static uint64_t readHpetRegister(uint64_t offset)
{
    return *(volatile uint64_t *)(hpetBase + offset);
}

__attribute__((always_inline))
static void writeHpetRegister(uint64_t offset, uint64_t val)
{
    *(volatile uint64_t *)(hpetBase + offset) = val;
}

static int enableHpet(void)
{
    int ret;
    uint64_t gcr = readHpetRegister(HPET_GCR_OFFSET);
    ret = !!(gcr & (1llu << 0));
    gcr |= (1llu << 0);
    writeHpetRegister(HPET_GCR_OFFSET, gcr);
    return ret;
}

static void disableHpet(void)
{
    uint64_t gcr = readHpetRegister(HPET_GCR_OFFSET);
    gcr &= ~(1llu << 0);
    writeHpetRegister(HPET_GCR_OFFSET, gcr);
}

static uint32_t getPeriod(void)
{
    uint64_t gcidr = readHpetRegister(HPET_GCIDR_OFFSET);
#if 0
    uint8_t revId, numTimCap, countSizeCap, legRtCap;
    uint16_t vendorId;
    uint32_t counterClkPeriod;
    revId = (gcidr >> 0) & 0xff;
    numTimCap = (gcidr >> 8) & 0x0f;
    countSizeCap = (gcidr >> 13) & 0x01;
    legRtCap = (gcidr >> 15) & 0x01;
    vendorId = (gcidr >> 16) & 0xffff;
    counterClkPeriod = (gcidr >> 32) & 0xffffffffu;
    DEBUG_PRINT("REV ID: %d\n", revId);
    DEBUG_PRINT("NUM TIM CAP: %d\n", numTimCap);
    DEBUG_PRINT("COUNT SIZE CAP: %d\n", countSizeCap);
    DEBUG_PRINT("LEG RT CAP: %d\n", legRtCap);
    DEBUG_PRINT("COUNTER CLK PERIOD: %d\n", counterClkPeriod);
#endif
    return (uint32_t)((gcidr >> 32) & 0xffffffffllu);
}

static void hpetInit(void)
{
    // find the HPET description table from the ACPI table
    MyOsHpetDescriptor *hpet =
        (MyOsHpetDescriptor *)findDescriptionTable(ACPI_SDT_HPET_SIG);
    //DEBUG_PRINT("hpet table address: 0x%016lx\n", (uint64_t)hpet);
    if (!hpet) {
        panic("hpet table not found\n");
    }
    hpetBase = P2V(hpet->baseAddress.address);
    DEBUG_PRINT("HPET base address: 0x%016lx\n", hpetBase);

    // disable HPET and Legacy replacement
    uint64_t gcr = readHpetRegister(HPET_GCR_OFFSET);
    gcr &= ~(1llu << 0);
    gcr &= ~(1llu << 1);
    writeHpetRegister(HPET_GCR_OFFSET, gcr);
}

void usleepBusy(uint64_t us)
{
#define US_TO_TICK(us) ((us)*1000000000llu/getPeriod())
    uint64_t cur = readHpetRegister(HPET_MCVR_OFFSET);
    uint64_t target = cur + US_TO_TICK(us);
    int isEnabled = enableHpet();

    while (readHpetRegister(HPET_MCVR_OFFSET) < target) {
        __asm__ __volatile__("pause");
    }
    if (!isEnabled) disableHpet();
}

void timerInit(void)
{
    DEBUG_PRINT("%s()\n", __func__);

    hpetInit();

#if 0
    printf("timer test\n");
    usleepBusy(5000000);
    printf("5sec elapsed\n");
#endif
}
