#ifndef MYOS_KERNEL_ACPI_H_
#define MYOS_KERNEL_ACPI_H_

#include <common/types.h>

#define ACPI_RSDP_SIG "RSD PTR "
#define ACPI_RSDP_SIG_LEN 8
#define ACPI_SDT_SIG_LEN 4
#define ACPI_OEM_ID_LEN 6
#define ACPI_OEM_TABLE_ID_LEN 8

typedef struct __attribute__((packed)) myos_acpi_rsdp_descriptor {
    struct __attribute__((packed)) {
        char signature[ACPI_RSDP_SIG_LEN];
        uint8_t checksum;
        char oemId[ACPI_OEM_ID_LEN];
        uint8_t revision;
        uint32_t rsdtAddress;
    } oldDesc;
    // below fields are added from ACPI Ver 2.0
    uint32_t length;
    uint64_t xsdtAddress;
    uint8_t extendedChecksum;
    uint8_t reserved[3];
} MyOsAcpiRsdpDescriptor;

typedef struct __attribute__((packed)) myos_acpi_sdt_header {
    char signature[ACPI_SDT_SIG_LEN];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oemId[ACPI_OEM_ID_LEN];
    char oemTableId[ACPI_OEM_TABLE_ID_LEN];
    uint32_t oemRevision;
    uint32_t creatorId;
    uint32_t creatorRevision;
} MyOsAcpiSdtHeader;

typedef struct __attribute__((packed)) myos_acpi_xsdt {
    MyOsAcpiSdtHeader header;
    uint64_t tables[];
} MyOsAcpiXsdt;

void acpiInit(MyOsAcpiRsdpDescriptor *rsdp);
int isValidAcpiChecksum(const void *data, size_t len);
MyOsAcpiSdtHeader *findDescriptionTable(uint8_t signature[ACPI_SDT_SIG_LEN]);

#endif // MYOS_KERNEL_ACPI_H_
