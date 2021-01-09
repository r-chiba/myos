#ifndef MYOS_KERNEL_ARCH_X64_GDT_H_
#define MYOS_KERNEL_ARCH_X64_GDT_H_

#include <arch/x64/descriptor.h>

#define KERNEL_TEXT_SEGMENT (1u<<3)
#define KERNEL_DATA_SEGMENT (2u<<3)
#define USER_TEXT_SEGMENT   (3u<<3)
#define USER_DATA_SEGMENT   (4u<<3)

void gdtInit(void);

#endif // MYOS_KERNEL_ARCH_X64_GDT_H_
