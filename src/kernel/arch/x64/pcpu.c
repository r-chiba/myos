#include <arch/x64/pcpu.h>
#include <arch/x64/constant.h>
#include <arch/x64/asmfunc.h>
#include <arch/x64/interrupt.h>

MyOsPerCpuData pcpu[MAXCPUNUM];

void pcpuInit(void)
{
    // set FSBASE and GSBASE and prohibit users from changing them
    // TODO: set per-cpu data to GSBASE
    writeMsr(MSR_FSBASE, 0);
    uint8_t id = getLocalApicId();
    memset(&pcpu[id], 0, sizeof(pcpu[id]));
    writeMsr(MSR_GSBASE, &pcpu[id]);
#if 0
    __asm__ __volatile__ (
            "movq   %%cr4, %%rax\n\t"
            "andq   %0, %%rax\n\t"
            "movq   %%rax, %%cr4"
            :: "i"(~CR4_FSGSBASE)
            : "rax");
#endif
}
