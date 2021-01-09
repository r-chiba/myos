#include <common/types.h>
#include <common/bootparam.h>
#include <util.h>
#include <graphics.h>
#include <acpi.h>

// architecture-dependent initialization routine
void archInit(void);

__attribute__((section(".text.entry")))
void kmain(MyOsBootParameter *bootparam)
{
    // printf can be used after the graphics initialization
    graphicsInit(&bootparam->graphicsInfo);
    printf("!!!myos kernel!!!\n");

    // archtecture-specific initialization routines may be use ACPI tables
    acpiInit((MyOsAcpiRsdpDescriptor *)bootparam->acpiTableAddr);

    archInit();

    printf("myos initialization done.\n");
    while (1) halt();
}

