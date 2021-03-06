#ifndef MYOS_KERNEL_UTIL_H_
#define MYOS_KERNEL_UTIL_H_

#include <common/util.h>

void panic(const char *fmt, ...);

#define KASSERT(pred, ...) \
do { \
    if (!(pred)) { \
        panic(__VA_ARGS__); \
    } \
} while (0)

#define offsetof(s,m) (size_t)&(((s *)NULL)->m)

#endif // MYOS_KERNEL_UTIL_H_
