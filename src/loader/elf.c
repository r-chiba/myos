#include <Uefi.h>
#include <freebsd/elf64.h>
#include <common/util.h>

extern EFI_SYSTEM_TABLE *gst;
#define BS (gst->BootServices)
#define CO (gst->ConOut)

static EFI_STATUS isSupportedElf(const Elf64_Ehdr *ehdr)
{
    if (!IS_ELF(*ehdr)) {
        printf("invalid magic\r\n");
        return EFI_INVALID_PARAMETER;
    }
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        printf("support only ELF64\r\n");
        return EFI_UNSUPPORTED;
    }
    if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        printf("support only little-endian\r\n");
        return EFI_UNSUPPORTED;
    }
    if (ehdr->e_ident[EI_VERSION] != EV_CURRENT
        || ehdr->e_version != EV_CURRENT) {
        printf("unsupported elf version\r\n");
        return EFI_UNSUPPORTED;
    }
    if (ehdr->e_ident[EI_OSABI] != ELFOSABI_SYSV) {
        printf("unsupported os or abi\r\n");
        return EFI_UNSUPPORTED;
    }
    if (ehdr->e_machine != EM_X86_64) {
        printf("unsupported machine\r\n");
        return EFI_UNSUPPORTED;
    }
    if (ehdr->e_ehsize != sizeof(Elf64_Ehdr)
        || ehdr->e_phentsize != sizeof(Elf64_Phdr)
        || ehdr->e_shentsize != sizeof(Elf64_Shdr)) {
        printf("structure size is invalid\r\n");
        return EFI_INVALID_PARAMETER;
    }

    if (ehdr->e_type != ET_DYN) {
        printf("support only PIE\r\n");
        return EFI_UNSUPPORTED;
    }

    printf("all passed\r\n");
    return EFI_SUCCESS;
}

EFI_STATUS loadSegment(
    EFI_PHYSICAL_ADDRESS addr,
    const Elf64_Ehdr *ehdr,
    const Elf64_Phdr *phdr)
{
    if (!IS_ALIGNED(addr, phdr->p_align)) {
        printf("inappropriate load address addr=0x%llx align=%u\r\n", addr, phdr->p_align);
        return EFI_INVALID_PARAMETER;
    }
    if (phdr->p_filesz > phdr->p_memsz) {
        printf("inappropriate format\r\n");
        return EFI_INVALID_PARAMETER;
    }
    UINT8 *content = (UINT8 *)ehdr + phdr->p_offset;
    memcpy(addr, content, phdr->p_filesz);
    if (phdr->p_filesz < phdr->p_memsz) {
        memset(addr+phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
    }
    return EFI_SUCCESS;
}

static EFI_STATUS loadSegments(
    const Elf64_Ehdr *ehdr,
    const Elf64_Phdr *phdr,
    UINT64 *loadaddr)
{
    EFI_STATUS status;
    size_t off = 0, nPages;
    EFI_PHYSICAL_ADDRESS addr;

    if (ehdr->e_phnum == 0) {
        printf("no program header\r\n");
        return EFI_UNSUPPORTED;
    }
    if (phdr[0].p_paddr != 0) {
        printf("start addr is not zero\r\n");
        return EFI_UNSUPPORTED;
    }

    // calculate memory size needed to load the elf
    // and allocate a memory region
	for (int i = 0; i < ehdr->e_phnum; i++) {
		if (phdr[i].p_type != PT_LOAD
            || phdr[i].p_memsz == 0) {
            continue;
        }
        if (phdr[i].p_paddr != phdr[i].p_vaddr) {
            printf("not PIE?\r\n");
            return EFI_UNSUPPORTED;
        }
        off = ROUNDUP(off, phdr[i].p_align);
        off += phdr[i].p_memsz;
        printf("phdr[%d].p_memsz=%llu\r\n", i, phdr[i].p_memsz);
    }
    printf("off=%llu\r\n", off);
    nPages = ROUNDUP(off, PAGE_SIZE) >> PAGE_SHIFT;
    status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, nPages, &addr);
    if (EFI_ERROR(status)) {
        printf("can't allocate pages\r\n");
        return status;
    }

    // load each segment
    off = 0;
	for (int i = 0; i < ehdr->e_phnum; i++) {
		if (phdr[i].p_type != PT_LOAD
            || phdr[i].p_memsz == 0) {
            continue;
        }
        off = ROUNDUP(off, phdr[i].p_align);
        status = loadSegment(addr+off, ehdr, &phdr[i]);
        if (EFI_ERROR(status)) {
            BS->FreePages(addr, nPages);
            return status;
        }
        off += phdr[i].p_memsz;
    }
    *loadaddr = addr;
    return EFI_SUCCESS;
}

EFI_STATUS loadElf(const UINT8 *file, UINT64 *entrypoint)
{
    EFI_STATUS status;
    const Elf64_Ehdr *ehdr;
	const Elf64_Phdr *phdr;
    UINT64 loadaddr;

    if (!file || !entrypoint) {
        return EFI_INVALID_PARAMETER;
    }

    ehdr = (const Elf64_Ehdr *)file;
    status = isSupportedElf(ehdr);
    if (EFI_ERROR(status)) {
        printf("isSupportedElf() error\r\n");
        return status;
    }

	phdr = (const Elf64_Phdr *)(file + ehdr->e_phoff); 

    status = loadSegments(ehdr, phdr, &loadaddr);
    if (EFI_ERROR(status)) {
        printf("loadSegments() error\r\n");
        return status;
    }
    *entrypoint = loadaddr + ehdr->e_entry;
    return EFI_SUCCESS;
}

