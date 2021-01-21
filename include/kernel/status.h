#ifndef MYOS_KERNEL_STATUS_H_
#define MYOS_KERNEL_STATUS_H_

typedef int MYOS_STATUS;

#define MYOS_OK 0
#define MYOS_ERROR(n) ((n) != MYOS_OK)

#define MYOS_ENOENT 1
#define MYOS_EINVAL 2
#define MYOS_ENOTSUP 3
#define MYOS_EFORMAT 4
#define MYOS_ENOMEM 5

#endif // MYOS_KERNEL_STATUS_H_
