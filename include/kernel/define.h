#ifndef MYOS_KERNEL_DEFINE_H_
#define MYOS_KERNEL_DEFINE_H_

#define static_assert _Static_assert

#define alloca(size) __builtin_alloca((size))
#define va_start(ap, last) __builtin_va_start((ap), (last))
#define va_arg __builtin_va_arg
#define va_end(ap) __builtin_va_end((ap))
#define va_copy(dest, src) __builtin_va_copy((dest), (src))
typedef __builtin_va_list va_list;

#endif // MYOS_KERNEL_DEFINE_H_
