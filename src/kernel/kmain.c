#include <common/types.h>
#include <common/bootparam.h>
#include <util.h>
#include <graphics.h>
#include <acpi.h>

// architecture-specific initialization routine prototype
void archInit(MyOsBootParameter *bootparam);
void halt(void);

__attribute__((section(".text.entry")))
void kmain(MyOsBootParameter *bootparam)
{
    // printf can be used after the graphics initialization
    graphicsInit(&bootparam->graphicsInfo);
    printf("!!!myos kernel!!!\n");

    // archtecture-specific initialization routines may use ACPI tables
    acpiInit((MyOsAcpiRsdpDescriptor *)bootparam->acpiTableAddr);

    archInit(bootparam);

    printf("!!!myos kernel initialization done!!!\n");
    while (1) halt();
}

