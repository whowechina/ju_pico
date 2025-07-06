#ifndef HUB75_H
#define HUB75_H

#include <stdint.h>
extern uint32_t canvas[64][64];

void hub75_init(bool fm6126);
void hub75_update();
uint16_t hub75_width();
uint16_t hub75_height();

static inline void hub75_clear()
{
    memset(canvas, 0, sizeof(canvas));
}

static inline uint32_t hub75_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | b;
}

// no memory protecting, so use with care
static inline void hub75_pixel(int x, int y, uint32_t color)
{
    canvas[y][x] = color;
}

#endif
