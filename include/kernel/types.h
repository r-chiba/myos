#ifndef MYOS_KERNEL_TYPES_H_
#define MYOS_KERNEL_TYPES_H_

//#if __LP64__ != 1
//#error unsupported environment
//#endif

#define NULL ((void *)0)
#define CHAR_BIT (__CHAR_BIT__)
#define LONG_BIT (__CHAR_BIT__*__SIZEOF_LONG__)

typedef __INT8_TYPE__ int8_t;
typedef __INT16_TYPE__ int16_t;
typedef __INT32_TYPE__ int32_t;
typedef __INT64_TYPE__ int64_t;
typedef __INTMAX_TYPE__ intmax_t;
#define INT8_MAX (int8_t)(__INT8_MAX__)
#define INT8_MIN (int8_t)(-INT8_MAX-1)
#define INT16_MAX (int16_t)(__INT16_MAX__)
#define INT16_MIN (int16_t)(-INT16_MAX-1)
#define INT32_MAX (int32_t)(__INT32_MAX__)
#define INT32_MIN (int32_t)(-INT32_MAX-1)
#define INT64_MAX (int64_t)(__INT64_MAX__)
#define INT64_MIN (int64_t)(-INT64_MAX-1)
#define INTMAX_MAX (intmax_t)(__INTMAX_MAX__)
#define INTMAX_MIN (intmax_t)(-INTMAX_MAX-1)

typedef __UINT8_TYPE__ uint8_t;
typedef __UINT16_TYPE__ uint16_t;
typedef __UINT32_TYPE__ uint32_t;
typedef __UINT64_TYPE__ uint64_t;
typedef __UINTMAX_TYPE__ uintmax_t;
#define UINT8_MAX (uint8_t)(__UINT8_MAX__)
#define UINT16_MAX (uint16_t)(__UINT16_MAX__)
#define UINT32_MAX (uint32_t)(__UINT32_MAX__)
#define UINT64_MAX (uint64_t)(__UINT64_MAX__)
#define UINTMAX_MAX (uintmax_t)(__UINTMAX_MAX__)

typedef __UINTPTR_TYPE__ uintptr_t;
typedef __INTPTR_TYPE__ intptr_t;
typedef __SIZE_TYPE__ size_t;
typedef long int ssize_t;
typedef intptr_t ptrdiff_t;
typedef intptr_t off_t;

#endif // MYOS_KERNEL_TYPES_H_
