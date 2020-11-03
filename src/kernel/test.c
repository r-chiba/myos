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
#define BOOTINFO_SMAP_ENTRY_SIZE 24

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
    uint16_t nentries = *(uint16_t *)BOOTINFO_SMAP_NENTRIES_ADDR;
    uint8_t *smap = (uint8_t *)BOOTINFO_SMAP_START_ADDR;
    printf("!!!myos kernel!!!\n");
    printf("physical memory map (e820):\n");
    //printf("Base             Size             Type\n");
    for (int i = 0; i < nentries; i++) {
        uint8_t *entry = smap + i*BOOTINFO_SMAP_ENTRY_SIZE;
        uint64_t base = *(uint64_t *)entry;
        uint64_t size = *(uint64_t *)(entry + sizeof(uint64_t));
        uint32_t type = *(uint32_t *)(entry + sizeof(uint64_t)*2);
        printf("[%016lx-%016lx] %s\n", base, base+size-1, (type == 1 ? "usable" : "reserved"));
    }
}

