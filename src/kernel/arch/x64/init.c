#include <arch/x64/gdt.h>
#include <arch/x64/interrupt.h>
#include <arch/x64/pmem.h>
#include <arch/x64/asminline.h>

void archInit(MyOsBootParameter *bootparam)
{
    gdtInit();
    interruptInit();
    pmemInit(&bootparam->memoryMapInfo);
    pcpuInit();
}

