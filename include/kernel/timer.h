#ifndef MYOS_KERNEL_TIMER_H_
#define MYOS_KERNEL_TIMER_H_

#include <common/types.h>
#include <status.h>
#include <acpi.h>

// signature of the ACPI Multiple APIC Description Table
#define ACPI_SDT_HPET_SIG "HPET"

#define HPET_GCIDR_OFFSET       0x00
#define HPET_GCR_OFFSET         0x10
#define HPET_MCVR_OFFSET        0xf0
#define HPET_TCCR_OFFSET(n)     (0x100+(n)*0x20)
#define HPET_TCVR_OFFSET(n)     (0x108+(n)*0x20)
#define HPET_TFIRR_OFFSET(n)    (0x110+(n)*0x20)

typedef struct __attribute__((packed)) myos_hpet_descriptor {
    MyOsAcpiSdtHeader header;
    uint32_t eventTimerBlockId;
    MyOsAcpiGenericAddress baseAddress;
    uint8_t hpetNumber;
    uint16_t mainCounterMinimumClockTick;
    uint8_t pageProtectionAndOemAttribute;
} MyOsHpetDescriptor;


void timerInit(void);

#endif // MYOS_KERNEL_ACPI_H_
