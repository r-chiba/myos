#ifndef MYOS_COMMON_DEFINE_H_
#define MYOS_COMMON_DEFINE_H_

#include <common/types.h>

#define static_assert _Static_assert

#define alloca(size) __builtin_alloca((size))
#define va_start(ap, last) __builtin_va_start((ap), (last))
#define va_arg __builtin_va_arg
#define va_end(ap) __builtin_va_end((ap))
#define va_copy(dest, src) __builtin_va_copy((dest), (src))
typedef __builtin_va_list va_list;

#define PAGE_SHIFT (12)
#define PAGE_SIZE ((size_t)1 << PAGE_SHIFT)

#if defined(KERNEL) && (KERNEL == 1)
#include <config.h>
#if !defined(CONFIG_PAGE_SIZE)
#error inappropriate configration
#endif
static_assert(PAGE_SIZE == CONFIG_PAGE_SIZE, "page size mismatch");
#endif

#endif // MYOS_COMMON_DEFINE_H_
