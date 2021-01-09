#include <arch/x64/gdt.h>
#include <arch/x64/interrupt.h>

void archInit(void)
{
    gdtInit();
    interruptInit();
}

