#ifndef MYOS_COMMON_BOOTPARAM_H_
#define MYOS_COMMON_BOOTPARAM_H_

#include <common/types.h>

typedef struct __attribute__((packed)) myos_graphics_info {
    uint64_t frameBufferBase;
    uint64_t frameBufferSize;
    uint32_t horizontalResolution;
    uint32_t verticalResolution;
    uint32_t pixelFormat;
    uint32_t pixelsPerScanLine;
} MyOsGraphicsInfo;

typedef struct __attribute__((packed)) myos_memory_map_info {
    uint64_t mapBase;
    uint64_t mapSize;
    uint64_t entrySize;
} MyOsMemoryMapInfo;

// must be same as EFI_MEMORY_DESCRIPTOR
typedef struct /*__attribute__((packed))*/ myos_memory_map_entry {
  uint32_t type;
  uint64_t physStart;
  uint64_t virtStart;
  uint64_t nPages;
  uint64_t attr;
} MyOsMemoryMapEntry;

typedef struct __attribute__((packed)) myos_vmem_info {
    void *archInfo; // pointer to an architecture-dependent structure
} MyOsVmemInfo;

typedef struct __attribute__((packed)) myos_boot_parameter {
    MyOsVmemInfo vmemInfo;
    uint64_t kernelLoadAddr;
    uint64_t kernelSize;
    uint64_t acpiTableAddr; // address of the ACPI Root System Description Pointer
    MyOsGraphicsInfo graphicsInfo;
    MyOsMemoryMapInfo memoryMapInfo;
} MyOsBootParameter;

#endif // MYOS_COMMON_BOOTPARAM_H_
