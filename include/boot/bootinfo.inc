; boot info:
;   offset (size)
;   + 0x00 (1)     : drive no. of the boot drive
;   + 0x02 (1)     : # of sectors of the boot drive
;   + 0x04 (1)     : # of heads of the boot drive
;   + 0x06 (2)     : # of tracks of the boot drive
;   + 0x10 (2)     : # of system memory map entries
;   + 0x20 (24*n)  : system memory map entries
%define BOOTINFO_START_ADDR     0x8000
%define BOOTINFO_DRIVE_NO_ADDR      (BOOTINFO_START_ADDR+0x00)
%define BOOTINFO_SECTORS_ADDR       (BOOTINFO_START_ADDR+0x02)
%define BOOTINFO_HEADS_ADDR         (BOOTINFO_START_ADDR+0x04)
%define BOOTINFO_TRACKS_ADDR        (BOOTINFO_START_ADDR+0x06)
%define BOOTINFO_SMAP_NENTRIES_ADDR (BOOTINFO_START_ADDR+0x10)
%define BOOTINFO_SMAP_START_ADDR    (BOOTINFO_START_ADDR+0x20)
