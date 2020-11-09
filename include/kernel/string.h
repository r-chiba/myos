#ifndef MYOS_KERNEL_STRING_H_
#define MYOS_KERNEL_STRING_H_

#include <kernel/define.h>
#include <kernel/types.h>

int printf(const char *fmt, ...);
int printf(const char *fmt, ...);
int sprintf(char *buf, const char *cfmt, ...);
int vprintf(const char *fmt, va_list ap);
size_t strlen(const char *str);
void *memset(void *dst0, int c0, size_t length);
void *memcpy(void *dst0, const void *src0, size_t length);

#endif // MYOS_KERNEL_STRING_H_
