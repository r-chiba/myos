#ifndef MYOS_COMMON_UTIL_H_
#define MYOS_COMMON_UTIL_H_

#include <common/types.h>
#include <common/string.h>

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
#define IS_ALIGNED(x, a) (((x) & ((a)-1)) == 0)
#define ROUNDUP(x, a) (((x) + ((a)-1)) & ~((a)-1))
#define ROUNDDOWN(x, a) ((x) & ~((a)-1))

#endif // MYOS_COMMON_UTIL_H_
