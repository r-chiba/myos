#define DEFAULT_COLOR 0x07 // lightgray-on-black

typedef unsigned char u8;
typedef unsigned short u16;

void _kputs(const char *str, u8 color)
{
    volatile u8 *vmem = (volatile u8 *)0xb8000;
    const char *cur = str;
    while (*cur) {
        *vmem++ = *cur++;
        *vmem++ = color;
    }
}

void kputs(const char *str)
{
    _kputs(str, DEFAULT_COLOR);
}

void centry(void)
{
    char *msg = "Hello, World! This is the C World!";
    kputs(msg);
}
