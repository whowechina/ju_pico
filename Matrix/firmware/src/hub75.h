/*
   HUB75 driver
   This is modified from Pimoroni's HUB75 driver:
   https://github.com/pimoroni/pimoroni-pico
*/

#ifndef HUB75_H
#define HUB75_H

#include <stdint.h>
#include <stdbool.h>

#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64
#define PANEL_BIT_DEPTH 10

const static uint16_t GAMMA_10BIT[256] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 82, 84, 87, 89, 91, 94, 96, 98, 101, 103, 106, 109, 111, 114, 117,
    119, 122, 125, 128, 130, 133, 136, 139, 142, 145, 148, 151, 155, 158, 161, 164,
    167, 171, 174, 177, 181, 184, 188, 191, 195, 198, 202, 206, 209, 213, 217, 221,
    225, 228, 232, 236, 240, 244, 248, 252, 257, 261, 265, 269, 274, 278, 282, 287,
    291, 295, 300, 304, 309, 314, 318, 323, 328, 333, 337, 342, 347, 352, 357, 362,
    367, 372, 377, 382, 387, 393, 398, 403, 408, 414, 419, 425, 430, 436, 441, 447,
    452, 458, 464, 470, 475, 481, 487, 493, 499, 505, 511, 517, 523, 529, 535, 542,
    548, 554, 561, 567, 573, 580, 586, 593, 599, 606, 613, 619, 626, 633, 640, 647,
    653, 660, 667, 674, 681, 689, 696, 703, 710, 717, 725, 732, 739, 747, 754, 762,
    769, 777, 784, 792, 800, 807, 815, 823, 831, 839, 847, 855, 863, 871, 879, 887,
    895, 903, 912, 920, 928, 937, 945, 954, 962, 971, 979, 988, 997, 1005, 1014, 1023
};

extern uint32_t canvas[64][64];

void hub75_init(bool fm6126, bool inverted_stb);
void hub75_update();

typedef void (*hub75_dma_complete_cb)(uint16_t row, uint16_t bit);
void hub75_start(hub75_dma_complete_cb dma_complete);
void hub75_pause();
void hub75_resume();
bool hub75_is_paused();

uint16_t hub75_width();
uint16_t hub75_height();

void hub75_clear();
void hub75_fill(uint32_t rgb);

static inline uint32_t hub75_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
    return (a << 24) | (r << 16) | (g << 8) | b;
}

static inline uint32_t hub75_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return hub75_argb(0xff, r, g, b);
}

static inline uint32_t hub75_color(uint32_t rgb)
{
    uint32_t r = GAMMA_10BIT[(rgb >> 16) & 0xff];
    uint32_t g = GAMMA_10BIT[(rgb >> 8) & 0xff];
    uint32_t b = GAMMA_10BIT[rgb & 0xff];
    return (b << 20) | (g << 10) | r;
}

static inline void hub75_pixel(int x, int y, uint32_t rgb)
{
    if ((x < 0) ||( x >= PANEL_WIDTH) || (y < 0) || (y >= PANEL_HEIGHT)) {
        return;
    }
    canvas[y][x] = hub75_color(rgb);
}

uint32_t hub75_hsv2rgb(uint8_t h, uint8_t s, uint8_t v);

static inline uint32_t hub75_alpha(uint8_t alpha, uint32_t rgb)
{
    return (alpha << 24) | (rgb & 0x00ffffff);
}

static inline void hub75_blend(int x, int y, uint32_t argb)
{
    uint32_t r1 = GAMMA_10BIT[(argb >> 16) & 0xff];
    uint32_t g1 = GAMMA_10BIT[(argb >> 8) & 0xff];
    uint32_t b1 = GAMMA_10BIT[argb & 0xff];

    uint32_t alpha = (argb >> 24) & 0xff;

    if (alpha == 0) {
        return;
    } else if (alpha == 255) {
        canvas[y][x] = (b1 << 20) | (g1 << 10) | r1;
        return;
    }

    uint32_t r0 = canvas[y][x] & 0x3ff;
    uint32_t g0 = (canvas[y][x] >> 10) & 0x3ff;
    uint32_t b0 = (canvas[y][x] >> 20) & 0x3ff;

    uint32_t r_mix = (r1 * alpha + r0 * (255 - alpha)) >> 8;
    uint32_t g_mix = (g1 * alpha + g0 * (255 - alpha)) >> 8;
    uint32_t b_mix = (b1 * alpha + b0 * (255 - alpha)) >> 8;
    canvas[y][x] = (b_mix << 20) | (g_mix << 10) | r_mix;
}


#endif
