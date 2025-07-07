/*
   HUB75 driver
   This is modified from Pimoroni's HUB75 driver:
   https://github.com/pimoroni/pimoroni-pico
*/

#ifndef HUB75_H
#define HUB75_H

#include <stdint.h>
#include <stdbool.h>

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

void hub75_init(bool fm6126, bool invert_stb);
void hub75_update();

typedef void (*hub75_dma_complete_cb)(uint row, uint bit);
void hub75_start(hub75_dma_complete_cb dma_complete);
void hub75_stop();

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
static inline void hub75_pixel(int x, int y, uint32_t rgb888)
{
    uint32_t r = (rgb888 >> 16) & 0xff;
    uint32_t g = (rgb888 >> 8) & 0xff;
    uint32_t b = rgb888 & 0xff;
    r = GAMMA_10BIT[r];
    g = GAMMA_10BIT[g];
    b = GAMMA_10BIT[b];
    canvas[y][x] = b << 20 | g << 10 | r;
}

#endif
