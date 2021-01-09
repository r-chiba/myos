#include <graphics.h>
#include <util.h>

static MyOsGraphicsInfo graphicsInfo;

typedef struct {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
  uint8_t reserved;
} PixelColor;

static PixelColor black = { 0x0, 0x0, 0x0, 0x0 };
static PixelColor white = { 0xff, 0xff, 0xff, 0x0 };

#define BACKGROUND_COLOR (&black)
#define TEXT_COLOR (&white) 

void graphicsInit(MyOsGraphicsInfo *bootGraphicsInfo)
{
    memcpy(&graphicsInfo, bootGraphicsInfo, sizeof(graphicsInfo));
}

static void drawPixel(uint32_t x, uint32_t y, PixelColor *c)
{
    uint32_t h = graphicsInfo.pixelsPerScanLine;
    PixelColor *base = (PixelColor *)graphicsInfo.frameBufferBase;
    PixelColor *p = base + h*y + x;
    p->blue = c->blue;
    p->green = c->green;
    p->red = c->red;
    p->reserved = c->reserved;
}

static void myPutChar(uint32_t x, uint32_t y, int c)
{
    if (c >= 0xff) return;
    for (int i = 0; i < FONT_HEIGHT; i++) {
        for (int j = 0; j < FONT_WIDTH; j++) {
            drawPixel(x+j, y+i,
                ((font[c][i] & (1<<j)) ? TEXT_COLOR : BACKGROUND_COLOR));
        }
    }
}

void putchar(int c)
{
    static uint32_t x = 0, y = 0;
    if (c >= 0xff) c = '#';
    if (c == '\r') return;
    if (c == '\n') {
        y += FONT_HEIGHT;
        x = 0;
    } else {
        myPutChar(x, y, c);
        x += FONT_WIDTH;
        if (x >= graphicsInfo.horizontalResolution) {
            y += FONT_HEIGHT;
            x = 0;
        }
        if (y + FONT_HEIGHT >= graphicsInfo.verticalResolution) {
            y = 0;
        }
    }
}

