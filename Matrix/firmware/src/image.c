/*
 * Image Plotter with Frame Support
 * WHowe <github.com/whowechina>
 */

#include <stdint.h>
#include "image.h"
#include "hub75.h"

static inline uint32_t pixel_color(const image_t *img, uint16_t x, uint16_t y)
{
    uint32_t pixel = y * img->width + x;

    if (img->color_depth == 24) {
        return ((uint32_t*)img->image)[pixel];
    }
    
    uint8_t pal_index;
    if (img->color_depth == 4) {
        uint32_t byte = pixel / 2;
        if (pixel & 1) {
            pal_index = img->image[byte] & 0x0f;
        } else {
            pal_index = img->image[byte] >> 4;
        }
    } else {
        pal_index = img->image[pixel];
    }
    
    if (!img->palette || (pal_index >= (1 << img->color_depth))) {
        return 0;
    }

    return img->palette[pal_index];
}

static inline uint8_t extract_alpha(uint8_t bit_per_pixel, uint8_t byte, int sector)
{
    switch (bit_per_pixel) {
        case 1:
            return (byte >> (7 - sector)) & 0x01 ? 0xff : 0x00;
        case 2:
            return ((byte >> (6 - sector * 2)) & 0x03) * 85;
        case 4:
            return ((byte >> (4 - sector * 4)) & 0x0f) * 17;
        case 8:
            return byte;
        default:
            return 0;
    }
}

static inline int pixel_alpha(const image_t *img, uint8_t x, uint8_t y)
{
    if (img->alpha_depth == 0) {
        return 255;
    }

    int pixels = y * img->width + x;
    int pixel_per_byte = 8 / img->alpha_depth;
    int byte = pixels / pixel_per_byte;
    int section = pixels % pixel_per_byte;

    return extract_alpha(img->alpha_depth, img->alpha[byte], section);
}

void image_draw(int x, int y, const image_t *img, unsigned int frame)
{
    if (!img) {
        return;
    }

    uint8_t frame_num = img->frame_num == 0 ? 1 : img->frame_num;

    if (frame >= frame_num) {
        return;
    }

    uint16_t frame_height = img->height / frame_num;
    uint16_t frame_offset = frame * frame_height;

    for (uint16_t dy = 0; dy < frame_height; dy++) {
        for (uint16_t dx = 0; dx < img->width; dx++) {
            uint32_t color = pixel_color(img, dx, frame_offset + dy);
            if ((img->alpha_depth == 0) || img->alpha) {
                uint8_t alpha = pixel_alpha(img, dx, frame_offset + dy);
                color = hub75_alpha(alpha, color);
            }

            hub75_blend(x + dx, y + dy, color);
        }
    }
}
