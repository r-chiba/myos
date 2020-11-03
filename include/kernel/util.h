#ifndef MYOS_KERNEL_UTIL_H_
#define MYOS_KERNEL_UTIL_H_

#include <kernel/types.h>

#define static_assert _Static_assert

#define alloca(size) __builtin_alloca((size))
#define va_start(ap, last) __builtin_va_start((ap), (last))
#define va_arg __builtin_va_arg
#define va_end(ap) __builtin_va_end((ap))
#define va_copy(dest, src) __builtin_va_copy((dest), (src))
typedef __builtin_va_list va_list;

//int vsnprintk(char *buf, size_t size, const char *format, va_list ap);
//int snprintk(char *buf, size_t size, const char *format, ...);
int printf(const char *fmt, ...);
size_t strlen(const char *str);

#endif // MYOS_KERNEL_UTIL_H_
