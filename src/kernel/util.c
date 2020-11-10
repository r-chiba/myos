#include <kernel/util.h>

void panic(const char *fmt, ...)
{
    printf("[PANIC] ");
	va_list ap;
	va_start(ap, fmt);
	(void)vprintf(fmt, ap);
	va_end(ap);

    while (1) {
        halt();
    }
}

int log2(uint64_t val)
{
    for (int i = 0; i < 63; i++) {
        if ((1UL << i) == val) {
            return i;
        }
    }
    DEBUG_PRINT("invalid value: %lu\n", val);
    return 0;
}
