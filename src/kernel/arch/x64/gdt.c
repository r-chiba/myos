#include <arch/x64/gdt.h>
#include <util.h>

static const uint64_t gdt[] __attribute__((section(".gdt"))) = {
    0x0,                // null descriptor
    0x00af9a000000ffff, // 64-bit kernel text segment
    0x00cf92000000ffff, // 64-bit kernel data segment
    // TODO: 64-bit user text segment
    // TODO: 64-bit user data segment
    // TODO: TSS for each processor
};

static const MyOsDescriptorRegister gdtr = {
    sizeof(gdt)-1,
    (uint64_t)&gdt
};

static void loadGdt(void)
{
    __asm__ __volatile__ (
            "lgdt %0\n\t"
            :: "m"(gdtr));
}

static void setSelectors(void)
{
    __asm__ __volatile__ (
            "movl   %0, %%eax\n\t"
            "movw   %%ax, %%ds\n\t"
            "movw   %%ax, %%es\n\t"
            "movw   %%ax, %%fs\n\t"
            "movw   %%ax, %%gs\n\t"
            // memo: external interrupts and debug exceptions
            //       are disabled after completion of an instruction
            //       immediately following the MOV SS instruction
            // should I care about something?
            "movw   %%ax, %%ss\n\t"
            "nop\n\t"

            "pushq  %1\n\t"
            "movq   $1f, %%rax\n\t"
            "pushq  %%rax\n\t"
            "lretq\n\t"
            "1:\n\t"
            :: "i"(KERNEL_DATA_SEGMENT), "i"(KERNEL_TEXT_SEGMENT)
            : "rax");
}

void gdtInit(void)
{
    DEBUG_PRINT("%s()\n", __func__);
    loadGdt();
    setSelectors();
    DEBUG_PRINT("gdt size:0x%04x, addr:0x%016x\n", 
                gdtr.descriptorTableLimit,
                gdtr.descriptorTableAddress);
}

