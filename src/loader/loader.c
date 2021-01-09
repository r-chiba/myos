#include <Uefi.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DevicePathFromText.h>
#include <Protocol/DevicePathToText.h>
#include <Protocol/GraphicsOutput.h>
#include <Guid/Acpi.h>
#include <common/bootparam.h>
#include <common/util.h>

static_assert(sizeof(INT8) == sizeof(int8_t), "type mismatch\r\n");
static_assert(sizeof(INT16) == sizeof(int16_t), "type mismatch\r\n");
static_assert(sizeof(INT32) == sizeof(int32_t), "type mismatch\r\n");
static_assert(sizeof(INT64) == sizeof(int64_t), "type mismatch\r\n");
static_assert(sizeof(INTN) == sizeof(int64_t), "type mismatch\r\n");
static_assert(sizeof(UINT8) == sizeof(uint8_t), "type mismatch\r\n");
static_assert(sizeof(UINT16) == sizeof(uint16_t), "type mismatch\r\n");
static_assert(sizeof(UINT32) == sizeof(uint32_t), "type mismatch\r\n");
static_assert(sizeof(UINTN) == sizeof(uint64_t), "type mismatch\r\n");

EFI_STATUS loadElf(const UINT8 *file, UINT64 *entrypoint);

#define LOAD_ELF 0
#if LOAD_ELF
#define KERNEL_FILE L"\\kernel.elf"
#else
#define KERNEL_FILE L"\\kernel.bin"
#endif

EFI_SYSTEM_TABLE *gst;
#define BS (gst->BootServices)
#define CO (gst->ConOut)

static EFI_GUID efiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static EFI_GUID efiSimpleFileSystemProtocolGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
static EFI_GUID efiDevicePathFromTextProtocolGuid = EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL_GUID;
static EFI_GUID efiDevicePathToTextProtocolGuid = EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;
static EFI_GUID efiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
static EFI_GUID efiAcpiTableGuid = EFI_ACPI_TABLE_GUID;

extern const uint8_t font[0xff][16];

typedef void (*MyOsKernelEntryPoint)(MyOsBootParameter *);

#if 0
// for printf
void putchar(int c)
{
    CHAR16 buf[2];
    buf[0] = (CHAR16)c;
    buf[1] = 0;
    CO->OutputString(CO, (CHAR16 *)buf);
}
#endif

#if 1
EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
void drawPixel(
    IN UINT32 x,
    IN UINT32 y,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL c)
{
    //UINT32 h = gop->Mode->Info->HorizontalResolution;
    UINT32 h = gop->Mode->Info->PixelsPerScanLine;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *base =
        (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gop->Mode->FrameBufferBase;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *p = base + h*y + x;
    p->Blue = c.Blue;
    p->Green = c.Green;
    p->Red = c.Red;
    p->Reserved = c.Reserved;
}
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL black = { 0x0, 0x0, 0x0, 0x0 };
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL white = { 0xff, 0xff, 0xff, 0x0 };

void myPutChar(
    UINT32 x,
    UINT32 y,
    int c)
{
#define FONT_HEIGHT 16
#define FONT_WIDTH 8
    if (c >= 0xff) return;
    for (int i = 0; i < FONT_HEIGHT; i++) {
        for (int j = 0; j < FONT_WIDTH; j++) {
            drawPixel(x+j, y+i,
                ((font[c][i] & (1<<j)) ? white : black));
        }
    }
}
#endif
void putchar(int c)
{
    static uint32_t x = 0, y = 0;
    if (c >= 0xff) c = '#';
    if (c == '\r') return;
    if (c == '\n') {
        y += FONT_HEIGHT;
        x = 0;
    } else {
        myPutChar(x, y, c);
        x += FONT_WIDTH;
        if (x >= gop->Mode->Info->HorizontalResolution) {
            y += FONT_HEIGHT;
            x = 0;
        }
    }
}

static EFI_STATUS readFile(
    IN EFI_FILE_PROTOCOL *root,
    IN CHAR16 *path,
    OUT UINT8 **buf,
    OUT UINTN *size)
{
    EFI_STATUS status;
    EFI_FILE_PROTOCOL *hfile;
    UINT64 fsz;
    UINTN rsz;

    if (!root || !path || !buf || !size) {
        return EFI_INVALID_PARAMETER;
    }
    status = root->Open(root, &hfile, path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        printf("Open() error\r\n");
        return status;
    }

    // get the kernel file size
    status = hfile->SetPosition(hfile, 0xffffffffffffffff);
    if (EFI_ERROR(status)) {
        printf("SetPosition() error\r\n");
        return status;
    }
    status = hfile->GetPosition(hfile, &fsz);
    if (EFI_ERROR(status)) {
        printf("GetPosition() error\r\n");
        return status;
    }
    status = hfile->SetPosition(hfile, 0);
    if (EFI_ERROR(status)) {
        printf("SetPosition() error\r\n");
        return status;
    }

    status = BS->AllocatePool(EfiLoaderData, fsz, (void **)buf);
    if (EFI_ERROR(status)) {
        printf("AllocatePool() error\r\n");
        return status;
    }
    rsz = fsz;
    status = hfile->Read(hfile, &rsz, *buf);
    if (EFI_ERROR(status) || rsz != fsz) {
        printf("Read() error %d, %lu %lu\r\n", status, fsz, rsz);
        BS->FreePool(buf);
        return status;
    }
    *size = rsz;

    status = hfile->Close(hfile);
    if (EFI_ERROR(status)) {
        printf("Close() error\r\n");
        BS->FreePool(buf);
        return status;
    }
    return EFI_SUCCESS;
}

static EFI_STATUS findAcpiTable(UINT64 *addr)
{
    if (!addr) {
        return EFI_INVALID_PARAMETER;
    }

    for (UINTN i = 0; i < gst->NumberOfTableEntries; i++) {
        EFI_CONFIGURATION_TABLE *table = &gst->ConfigurationTable[i];
        if (memcmp((void *)&table->VendorGuid, (void *)&efiAcpiTableGuid, sizeof(EFI_GUID)) == 0) {
            *addr = (UINT64)table->VendorTable;
            //printf("0x%016llx, 0x%016llx\r\n", *addr, (UINT64)table->VendorTable);
            //while (1);
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND;
}

EFI_STATUS EFIAPI UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS status;
    EFI_MEMORY_DESCRIPTOR *map = NULL;
    UINTN mapsz = 0, mapkey, descsz;
    UINT32 descver;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfsp = NULL;
    EFI_FILE_PROTOCOL *root;
    UINT8 *kernel;
    UINTN ksz;
    UINT64 entrypointAddr;
    MyOsKernelEntryPoint entrypoint;
    EFI_LOADED_IMAGE_PROTOCOL *lip = NULL;
    EFI_DEVICE_PATH_FROM_TEXT_PROTOCOL *dpftp = NULL;
    EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *dpttp = NULL;
    //EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
    MyOsBootParameter *bootparam = NULL;

    gst = SystemTable;

    // prepare printf
    CO->ClearScreen(CO);
    status = BS->LocateProtocol(
                    &efiGraphicsOutputProtocolGuid,
                    NULL,
                    (void **)&gop);
    if (EFI_ERROR(status)) {
        //printf("LocateProtocol error\r\n");
        while (1);
        return EFI_OUT_OF_RESOURCES;
    }
    //printf("pixel format: %d\r\n", gop->Mode->Info->PixelFormat);
    if (gop->Mode->Info->PixelFormat != PixelBlueGreenRedReserved8BitPerColor) {
        //printf("unsupported pixel format for now\r\n");
        return EFI_OUT_OF_RESOURCES;
    }

    // print a welcome message
    printf("Hello World from UEFI!\r\n");

    // read the kernel file
    status = BS->LocateProtocol(
                    &efiSimpleFileSystemProtocolGuid,
                    NULL,
                    (void **)&sfsp);
    if (EFI_ERROR(status)) {
        printf("LocateProtocol error\r\n");
        while (1);
        return EFI_OUT_OF_RESOURCES;
    }
    status = sfsp->OpenVolume(sfsp, &root);
    if (EFI_ERROR(status)) {
        printf("OpenVolume error\r\n");
        while (1);
        return EFI_OUT_OF_RESOURCES;
    }
    status = readFile(root, KERNEL_FILE, &kernel, &ksz);
    if (EFI_ERROR(status)) {
        printf("readFile error\r\n");
        while (1);
        return EFI_OUT_OF_RESOURCES;
    }
#if LOAD_ELF
    printf("magic: 0x%x %c %c %c, kernel size: %lu\r\n",
            kernel[0], kernel[1], kernel[2], kernel[3], ksz);
    status = loadElf(kernel, &entrypointAddr);
    if (EFI_ERROR(status)) {
        printf("loadElf() error\r\n");
        while (1);
        return EFI_OUT_OF_RESOURCES;
    }
    printf("loadElf() success 0x%016lx\r\n", entrypointAddr);
    entrypoint = (MyOsKernelEntryPoint)entrypointAddr;
#else
    {
        UINTN nPages = ROUNDUP(ksz, PAGE_SIZE) >> PAGE_SHIFT;
        EFI_PHYSICAL_ADDRESS mem = 0x10000;
        status = BS->AllocatePages(AllocateAddress, EfiLoaderData, nPages, &mem);
        if (EFI_ERROR(status)) {
            printf("can't place the kernel to 0x%016llx\r\n", (UINT64)mem);
            return EFI_OUT_OF_RESOURCES;
        }
        memcpy(mem, kernel, ksz);
        // assume the entrypoint is at beginning of the image
        entrypoint = (MyOsKernelEntryPoint)mem;
    }
#endif

    // prepare a boot parameter
    status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, (void **)&bootparam);
    if (EFI_ERROR(status)) {
        printf("AllocatePages() error\r\n");
        return EFI_OUT_OF_RESOURCES;
    }
    memset(bootparam, 0, sizeof(*bootparam));
    bootparam->graphicsInfo.frameBufferBase = 
        gop->Mode->FrameBufferBase;
    bootparam->graphicsInfo.frameBufferSize = 
        gop->Mode->FrameBufferSize;
    bootparam->graphicsInfo.horizontalResolution =
        gop->Mode->Info->HorizontalResolution;
    bootparam->graphicsInfo.verticalResolution =
        gop->Mode->Info->VerticalResolution;
    bootparam->graphicsInfo.pixelFormat =
        gop->Mode->Info->PixelFormat;
    bootparam->graphicsInfo.pixelsPerScanLine =
        gop->Mode->Info->PixelsPerScanLine;

    status = findAcpiTable(&bootparam->acpiTableAddr);
    if (EFI_ERROR(status)) {
        printf("ACPI table not found\r\n");
        while (1);
        return EFI_OUT_OF_RESOURCES;
    }

    CO->ClearScreen(CO);
    do {
        while (1) {
            status = BS->GetMemoryMap(&mapsz, map, &mapkey, &descsz, &descver);
            if (status == EFI_SUCCESS) break;
            if (status == EFI_BUFFER_TOO_SMALL) {
                if (map) BS->FreePool(map);
                status = BS->AllocatePool(EfiLoaderData, mapsz, (void **)&map);
                if (EFI_ERROR(status)) {
                    while (1);
                    return EFI_OUT_OF_RESOURCES;
                }
            } else if (EFI_ERROR(status)) {
                while (1);
                return EFI_OUT_OF_RESOURCES;
            }
        }
#if 0
        UINTN ndescs = mapsz / descsz;
        for (UINTN i = 0; i < ndescs; i++) {
            EFI_MEMORY_DESCRIPTOR *desc =
                (EFI_MEMORY_DESCRIPTOR *)((UINT8 *)map + i*descsz);
            printf("%08x %016lx %016lx %016x %016x\r\n",
                    desc->Type,
                    desc->PhysicalStart,
                    desc->VirtualStart,
                    desc->NumberOfPages,
                    desc->Attribute);
            if (i == 4) break;
            //printf("%03u %016lx %08x %016lx %016lx %016lx %016x",
            //        i,
            //        (UINT64)desc,
            //        desc->Type,
            //        desc->PhysicalStart,
            //        desc->VirtualStart,
            //        desc->NumberOfPages,
            //        desc->Attribute);
        }
        //while (1);
#endif
        status = BS->ExitBootServices(ImageHandle, mapkey);
        //printf("status=%u\r\n", status);
    } while (EFI_ERROR(status));

    //printf("go to kernel 0x%016lx\r\n", (UINT64)entrypoint);

    // call MyOS kernel entrypoint along with System V ABI
    // TODO: prepare a kernel stack
    __asm__ __volatile__ (
            "call *%1\n\t"
            :
            :"D"(bootparam), "r"(entrypoint)
            : "cc", "memory");

    // not come here
    printf("return from kernel\r\n");
    while (1);

    return status == EFI_SUCCESS ? EFI_SUCCESS : EFI_OUT_OF_RESOURCES;
}
