#ifndef MYOS_KERNEL_UTIL_H_
#define MYOS_KERNEL_UTIL_H_

#include <kernel/types.h>
#include <kernel/string.h>

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(...) \
do { \
    printf("[DEBUG](%s:%u) ", __FUNCTION__, __LINE__); \
    printf(__VA_ARGS__); \
} while (0)
#else
#define DEBUG_PRINT(...) 
#endif

// assume a is power of two
#define ROUNDUP(x, a) (((x) + ((a)-1)) & ~((a)-1))
#define ROUNDDOWN(x, a) ((x) & ~((a)-1))

void panic(const char *fmt, ...);
int log2(uint64_t val);

#define KASSERT(pred, ...) \
do { \
    if (!(pred)) { \
        panic(__VA_ARGS__); \
    } \
} while (0)

#endif // MYOS_KERNEL_UTIL_H_
