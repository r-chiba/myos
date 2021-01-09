#ifndef MYOS_COMMON_STRING_H_
#define MYOS_COMMON_STRING_H_

#include <common/define.h>
#include <common/types.h>

int printf(const char *fmt, ...);
int printf(const char *fmt, ...);
int sprintf(char *buf, const char *cfmt, ...);
int vprintf(const char *fmt, va_list ap);
size_t strlen(const char *str);
void *memset(void *dst0, int c0, size_t length);
void *memcpy(void *dst0, const void *src0, size_t length);
int memcmp(const void *s1, const void *s2, size_t n);

#endif // MYOS_COMMON_STRING_H_
