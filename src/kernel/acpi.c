#include <acpi.h>
#include <util.h>

MyOsAcpiRsdpDescriptor *gRsdp = NULL;

int isValidAcpiChecksum(const void *data, size_t len)
{
    uint8_t *p = (const uint8_t *)data;
    uint8_t sum = 0;

    for (int i = 0; i < len; i++) {
        sum += *(p+i);
    }
    return sum == 0;
}

static int isValidRsdp(MyOsAcpiRsdpDescriptor *rsdp)
{
    // check the signature
    if (memcmp(rsdp->oldDesc.signature, ACPI_RSDP_SIG, ACPI_RSDP_SIG_LEN) != 0) {
        printf("acpi rsdp magic not matched\n");
        return 0;
    }

    // check the checksum and extended checksum
    if (!isValidAcpiChecksum(&rsdp->oldDesc, sizeof(rsdp->oldDesc))) {
        printf("rsdp checksum not matched\n");
        return 0;
    }
    if (!isValidAcpiChecksum(rsdp, sizeof(*rsdp))) {
        printf("rsdp extended checksum not matched\n");
        return 0;
    }
    return 1;
}

MyOsAcpiSdtHeader *findDescriptionTable(const uint8_t signature[ACPI_SDT_SIG_LEN])
{
    if (!gRsdp) return NULL;
    if (!isValidRsdp(gRsdp)) {
        printf("invalid rsdp\n");
        return NULL;
    }
    MyOsAcpiXsdt *xsdt = (MyOsAcpiXsdt *)gRsdp->xsdtAddress;
    if (!isValidAcpiChecksum(xsdt, xsdt->header.length)) {
        printf("xsdt checksum not matched\n");
        return NULL;
    }
    uint32_t nTables = (xsdt->header.length - sizeof(*xsdt)) / sizeof(uint64_t);
    for (uint32_t i = 0; i < nTables; i++) {
        MyOsAcpiSdtHeader *table = (MyOsAcpiSdtHeader *)xsdt->tables[i];
        if (memcmp(table->signature, signature, ACPI_SDT_SIG_LEN) == 0) {
            return table;
        }
    }
    return NULL;
}

void acpiInit(MyOsAcpiRsdpDescriptor *rsdp)
{
    KASSERT(rsdp, "no acpi rsdp\n");
    printf("APIC RSDP addr:0x%016llx\n", (uint64_t)rsdp);
    gRsdp = rsdp;
}
