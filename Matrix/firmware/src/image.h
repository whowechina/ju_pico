/*
 * Image Plotter with Frame Support
 * WHowe <github.com/whowechina>
 */

#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t color_depth; /* 4, 8, or 24 */
    uint8_t alpha_depth; /* 0, 2, 4, 8 */
    uint16_t width;
    uint16_t height;
    uint16_t frame_num;
    const uint32_t *palette;
    const uint8_t *alpha;
    const uint8_t *image;
} image_t;

void image_draw(int x, int y, const image_t *img, int frame, uint8_t alpha, uint8_t rotate);


#endif
