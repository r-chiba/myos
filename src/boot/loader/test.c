#include <kernel/types.h>
#include <kernel/util.h>

#define DEFAULT_COLOR 0x07 // lightgray-on-black
#define VMEM_START_ADDR 0xb8000

#define BOOTINFO_START_ADDR 0x8000
#define BOOTINFO_DRIVE_NO_ADDR      (BOOTINFO_START_ADDR+0x00)
#define BOOTINFO_SECTORS_ADDR       (BOOTINFO_START_ADDR+0x02)
#define BOOTINFO_HEADS_ADDR         (BOOTINFO_START_ADDR+0x04)
#define BOOTINFO_TRACKS_ADDR        (BOOTINFO_START_ADDR+0x06)
#define BOOTINFO_SMAP_NENTRIES_ADDR (BOOTINFO_START_ADDR+0x10)
#define BOOTINFO_SMAP_START_ADDR    (BOOTINFO_START_ADDR+0x20)

ptrdiff_t vmem_cur = 0;

#if 1
typedef unsigned char u8;
typedef unsigned short u16;

void _kputs(const char *str, u8 color)
{
    volatile u8 *vmem = (volatile u8 *)0xb8000;
    const char *cur = str;
    while (*cur) {
        *vmem++ = *cur++;
        *vmem++ = color;
    }
}

void kputs(const char *str)
{
    _kputs(str, DEFAULT_COLOR);
}
#endif

void putchar(int c)
{
    if (c != '\n') {
        volatile uint8_t *vmem = (volatile uint8_t *)(VMEM_START_ADDR + vmem_cur);
        *vmem++ = (c & 0xff);
        *vmem++ = DEFAULT_COLOR;
        vmem_cur += 2;
    } else {
        ptrdiff_t w = (vmem_cur % 160);
        //vmem_cur += (w == 0 ? 0 : 160 - w);
        vmem_cur += 160 - w;
    }
}

void centry(void)
{
    uint16_t nentries = *(uint16_t *)BOOTINFO_SMAP_NENTRIES_ADDR;
    uint8_t *smap = (uint8_t *)BOOTINFO_SMAP_START_ADDR;
    printf("myos loader\n\n");
    printf("BOOTPARAMS:\n");
    printf("drive no.=0x%02x,# of sectors=0x%02x,# of heads=0x%02x,# of tracks=0x%04x\n",
            *(uint8_t *)BOOTINFO_DRIVE_NO_ADDR,
            *(uint8_t *)BOOTINFO_SECTORS_ADDR,
            *(uint8_t *)BOOTINFO_HEADS_ADDR,
            *(uint16_t *)BOOTINFO_TRACKS_ADDR);
    printf("physical memory map (e820):\n");
    //printf("Base             Size             Type\n");
    for (int i = 0; i < nentries; i++) {
        uint64_t base = *(uint64_t *)(smap + i*24);
        uint64_t size = *(uint64_t *)(smap + i*24 + sizeof(uint64_t));
        uint32_t type = *(uint32_t *)(smap + i*24 + sizeof(uint64_t)*2);
        printf("[%016lx-%016lx] %s\n", base, base+size-1, (type == 1 ? "usable" : "reserved"));
    }
#define LOADER_TEXT_START_ADDR 0x09000
#define KERNEL_TEXT_START_ADDR 0x10000
    printf("0x%016lx\n", *(uint64_t *)LOADER_TEXT_START_ADDR);
    printf("0x%016lx\n", *(uint64_t *)KERNEL_TEXT_START_ADDR);
}
