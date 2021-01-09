#ifndef MYOS_KERNEL_GRAPHICS_H_
#define MYOS_KERNEL_GRAPHICS_H_

#include <common/types.h>
#include <common/bootparam.h>
#include <font.h>

void graphicsInit(MyOsGraphicsInfo *bootGraphicsInfo);

#endif // MYOS_KERNEL_GRAPHICS_H_
