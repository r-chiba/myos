#ifndef MYOS_KERNEL_ARCH_X64_INTERRUPT_H_
#define MYOS_KERNEL_ARCH_X64_INTERRUPT_H_

#include <arch/x64/descriptor.h>
#include <acpi.h>

// 64-bit IDT Gate Descriptor
typedef struct __attribute__((packed)) idtGateDescriptor {
    uint16_t offsetLowLow;
    uint16_t segmentSelector;
    uint16_t attr;
    uint16_t offsetLowHigh;
    uint32_t offsetHigh;
    uint32_t reserved;
} MyOsIdtGateDescriptor;

#define NINTDESCS 256

#define TYPE_INTERRUPT_GATE 0xe
#define TYPE_TRAP_GATE      0xf

#define INTVEC_DE        0x00 // Divide Error
#define INTVEC_DB        0x01 // Debug Exception
#define INTVEC_NMI       0x02 // Nonmaskable Interrupt
#define INTVEC_BP        0x03 // Breakpoint
#define INTVEC_OF        0x04 // Overflow
#define INTVEC_BR        0x05 // BOUND Range Exceeded
#define INTVEC_UD        0x06 // Invalid Opcode
#define INTVEC_NM        0x07 // Device Not Available
#define INTVEC_DF        0x08 // Double Fault
#define INTVEC_TS        0x0a // Invalid TSS
#define INTVEC_NP        0x0b // Segment Not Present
#define INTVEC_SS        0x0c // Stack-Segment Fault
#define INTVEC_GP        0x0d // General Protection
#define INTVEC_PF        0x0e // Page Fault
#define INTVEC_USER_BASE 0x20
#define INTVEC_IRQ0      (INTVEC_USER_BASE +  0)
#define INTVEC_IRQ1      (INTVEC_USER_BASE +  1)
#define INTVEC_IRQ2      (INTVEC_USER_BASE +  2)
#define INTVEC_IRQ3      (INTVEC_USER_BASE +  3)
#define INTVEC_IRQ4      (INTVEC_USER_BASE +  4)
#define INTVEC_IRQ5      (INTVEC_USER_BASE +  5)
#define INTVEC_IRQ6      (INTVEC_USER_BASE +  6)
#define INTVEC_IRQ7      (INTVEC_USER_BASE +  7)
#define INTVEC_IRQ8      (INTVEC_USER_BASE +  8)
#define INTVEC_IRQ9      (INTVEC_USER_BASE +  9)
#define INTVEC_IRQ10     (INTVEC_USER_BASE + 10)
#define INTVEC_IRQ11     (INTVEC_USER_BASE + 11)
#define INTVEC_IRQ12     (INTVEC_USER_BASE + 12)
#define INTVEC_IRQ13     (INTVEC_USER_BASE + 13)
#define INTVEC_IRQ14     (INTVEC_USER_BASE + 14)
#define INTVEC_IRQ15     (INTVEC_USER_BASE + 15)

// signature of the ACPI Multiple APIC Description Table
#define ACPI_SDT_MADT_SIG "APIC"

// ACPI MADT structures
typedef struct __attribute__((packed)) myos_acpi_madt {
    MyOsAcpiSdtHeader header;
    uint32_t localApicAdress;
    uint32_t flags;
    uint8_t data[];
} MyOsAcpiMadt;

typedef struct __attribute__((packed)) myos_acpi_madt_entry_header {
    uint8_t entryType;
    uint8_t recordLength;
} MyOsAcpiMadtEntryHeader;

// MADT entry type 0
typedef struct __attribute__((packed)) myos_acpi_madt_entry_local_apic {
    MyOsAcpiMadtEntryHeader header;
    uint8_t acpiProcessorId;
    uint8_t apicId;
    uint32_t flags;
} MyOsAcpiMadtEntryLocalApic;

// MADT entry type 1
typedef struct __attribute__((packed)) myos_acpi_madt_entry_io_apic {
    MyOsAcpiMadtEntryHeader header;
    uint8_t ioApicId;
    uint8_t reserved;
    uint32_t ioApicAddress;
    uint32_t globalSystemInterruptBase;
} MyOsAcpiMadtEntryIoApic;

// MADT entry type 2
typedef struct __attribute__((packed)) myos_acpi_madt_entry_interrupt_source_override {
    MyOsAcpiMadtEntryHeader header;
    uint8_t busSource;
    uint8_t irqSource;
    uint32_t globalSystemInterrupt;
    uint16_t flags;
} MyOsAcpiMadtEntryInterruptSourceOverride;

// MADT entry type 4
typedef struct __attribute__((packed)) myos_acpi_madt_entry_nonmaskable_interrupts {
    MyOsAcpiMadtEntryHeader header;
    uint8_t acpiProcessorId;
    uint16_t flags;
    uint8_t lint;
} MyOsAcpiMadtEntryNonmaskableInterrupts;

// MADT entry type 5
typedef struct __attribute__((packed)) myos_acpi_madt_entry_local_apic_address_override {
    MyOsAcpiMadtEntryHeader header;
    uint16_t reserved;
    uint64_t localApicPhysAddr;
} MyOsAcpiMadtEntryLocalApicAddressOverride;

#define IOAPIC_IOREGSEL_OFFSET 0
#define IOAPIC_IOWIN_OFFSET 0x10
#define IOAPIC_IOREDTBL_LOW_OFFSET(n) (0x10+2*(n))
#define IOAPIC_IOREDTBL_HIGH_OFFSET(n) (0x10+2*(n)+1)

void interruptInit(void);

void defaultInterruptHandler(void);
void keyboardInterruptHandler(void);

#endif // MYOS_KERNEL_ARCH_X64_INTERRUPT_H_
