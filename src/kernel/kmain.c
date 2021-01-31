#include <common/types.h>
#include <common/bootparam.h>
#include <util.h>
#include <graphics.h>
#include <acpi.h>
#include <vmem.h>

// architecture-specific initialization routine prototype
void archInit(MyOsBootParameter *bootparam);
void halt(void);
extern char _kernel_start[], _kernel_end[];

void kmain(MyOsBootParameter *bootparam)
{
    bootparam = P2V(bootparam);

    graphicsInit(&bootparam->graphicsInfo);
    // printf can be used after the graphics initialization
    printf("!!!myos kernel!!!\n");
    printf("kernel area:%p-%p\n", _kernel_start, _kernel_end);

    // archtecture-specific initialization routines may use ACPI tables
    acpiInit((MyOsAcpiRsdpDescriptor *)bootparam->acpiTableAddr);

    archInit(bootparam);

    printf("!!!myos kernel initialization done!!!\n");
    while (1) halt();
}

