#ifndef MYOS_KERNEL_UTIL_H_
#define MYOS_KERNEL_UTIL_H_

#include <kernel/types.h>
#include <kernel/string.h>

// assume a is power of two
#define ROUNDUP(x, a) (((x) + ((a)-1)) & ~((a)-1))
#define ROUNDDOWN(x, a) ((x) & ~((a)-1))

void panic(const char *fmt, ...);

#define KASSERT(pred, ...) \
do { \
    if (!(pred)) { \
        panic(__VA_ARGS__); \
    } \
} while(0)

#ifdef DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...) 
#endif

#endif // MYOS_KERNEL_UTIL_H_
