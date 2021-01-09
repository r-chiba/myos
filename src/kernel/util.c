#include <util.h>
#include <arch/x64/asmfunc.h>

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

