#include <kernel/types.h>
#include <kernel/util.h>
#include <kernel/bootinfo.h>
#include <kernel/physmem.h>
#include <freebsd/sys/sys/queue.h>

#define DEFAULT_COLOR 0x07 // lightgray-on-black
#define VMEM_START_ADDR 0xb8000

ptrdiff_t vmem_cur = 0;

void putchar(int c)
{
    if (c != '\n') {
        volatile uint8_t *vmem = (volatile uint8_t *)(VMEM_START_ADDR + vmem_cur);
        *vmem++ = (c & 0xff);
        *vmem++ = DEFAULT_COLOR;
        vmem_cur += 2;
    } else {
        ptrdiff_t w = (vmem_cur % 160);
        vmem_cur += 160 - w;
    }
}

void kmain(void)
{
    printf("!!!myos kernel!!!\n");
    init_physmem();
}

