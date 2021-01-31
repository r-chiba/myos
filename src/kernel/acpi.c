#include <acpi.h>
#include <util.h>
#include <vmem.h>

MyOsAcpiRsdpDescriptor *gRsdp = NULL;

MYOS_STATUS isValidAcpiChecksum(const void *data, size_t len)
{
    uint8_t *p = (const uint8_t *)data;
    uint8_t sum = 0;

    for (int i = 0; i < len; i++) {
        sum += *(p+i);
    }
    return sum == 0 ? MYOS_OK : MYOS_EFORMAT;
}

static MYOS_STATUS isValidRsdp(MyOsAcpiRsdpDescriptor *rsdp)
{
    MYOS_STATUS status;

    // check the signature
    if (memcmp(rsdp->oldDesc.signature, ACPI_RSDP_SIG, ACPI_RSDP_SIG_LEN) != 0) {
        printf("acpi rsdp magic not matched\n");
        return MYOS_EFORMAT;
    }

    // check the checksum and extended checksum
    status = isValidAcpiChecksum(&rsdp->oldDesc, sizeof(rsdp->oldDesc));
    if (MYOS_ERROR(status)) {
        printf("rsdp checksum not matched\n");
        return MYOS_EFORMAT;
    }
    status = isValidAcpiChecksum(rsdp, sizeof(*rsdp));
    if (MYOS_ERROR(status)) {
        printf("rsdp extended checksum not matched\n");
        return MYOS_EFORMAT;
    }
    return MYOS_OK;
}

MyOsAcpiSdtHeader *findDescriptionTable(const uint8_t signature[ACPI_SDT_SIG_LEN])
{
    MYOS_STATUS status;

    if (!gRsdp) return NULL;
    status = isValidRsdp(gRsdp);
    if (MYOS_ERROR(status)) {
        printf("invalid rsdp\n");
        return NULL;
    }
    MyOsAcpiXsdt *xsdt = (MyOsAcpiXsdt *)P2V(gRsdp->xsdtAddress);
    status = isValidAcpiChecksum(xsdt, xsdt->header.length);
    if (MYOS_ERROR(status)) {
        printf("xsdt checksum not matched\n");
        return NULL;
    }
    uint32_t nTables = (xsdt->header.length - sizeof(*xsdt)) / sizeof(uint64_t);
    for (uint32_t i = 0; i < nTables; i++) {
        MyOsAcpiSdtHeader *table = (MyOsAcpiSdtHeader *)P2V(xsdt->tables[i]);
        if (memcmp(table->signature, signature, ACPI_SDT_SIG_LEN) == 0) {
            status = isValidAcpiChecksum(table, table->length);
            if (!MYOS_ERROR(status)) {
                return table;
            }
        }
    }
    return NULL;
}

void acpiInit(MyOsAcpiRsdpDescriptor *rsdp)
{
    DEBUG_PRINT("%s()\n", __func__);
    KASSERT(rsdp, "no acpi rsdp\n");
    rsdp = P2V(rsdp);
    DEBUG_PRINT("ACPI RSDP addr:0x%016llx\n", (uint64_t)rsdp);
    gRsdp = rsdp;
}

