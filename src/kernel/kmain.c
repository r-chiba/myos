#include <common/types.h>
#include <common/bootparam.h>
#include <util.h>
#include <graphics.h>
#include <acpi.h>
#include <vmem.h>
#include <timer.h>

// architecture-specific initialization routine prototype
void archInit(MyOsBootParameter *bootparam);
void archInitForAp(void);
void mpInit(void);
uint8_t getLocalApicId(void);
void halt(void);
extern char _kernel_start[], _kernel_end[];
extern uint64_t nAwakedCpus;

void kmain(MyOsBootParameter *bootparam)
{
    if (bootparam) { // I'm the BSP
        bootparam = P2V(bootparam);

        // printf can be used after the graphics initialization
        graphicsInit(&bootparam->graphicsInfo);
        printf("!!!myos kernel!!!\n");
        printf("kernel area:0x%016lx-0x%016lx\n", _kernel_start, _kernel_end);

        // archtecture-specific initialization routines may use ACPI tables
        acpiInit((MyOsAcpiRsdpDescriptor *)bootparam->acpiTableAddr);

        archInit(bootparam);

        timerInit();

        mpInit();

        printf("!!!myos kernel initialization done!!!\n");
    } else {
        printf("!!!AP %u awaked!!!\n", getLocalApicId());
        archInitForAp();
        nAwakedCpus++;
    }
    while (1) halt();
}

