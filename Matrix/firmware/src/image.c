/*
 * Image Plotter with Frame Support
 * WHowe <github.com/whowechina>
 */

#include <stdint.h>
#include "image.h"
#include "hub75.h"

static inline uint32_t pixel_color(const image_t *img, int x, int y)
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

static inline uint8_t extract_alpha(uint8_t bitwidth, uint8_t byte, int sector)
{
    switch (bitwidth) {
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

static inline int pixel_alpha(const image_t *img, int x, int y)
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

void image_draw(int x, int y, const image_t *img, int frame, uint8_t alpha, uint8_t rotate)
{
    if (!img) {
        return;
    }
    if (alpha == 0) {
        return;
    }

    int frame_num = img->frame_num <= 0 ? 1 : img->frame_num;

    if (frame >= frame_num) {
        return;
    }

    int frame_width = img->width;
    int frame_height = img->height / frame_num;
    int height_offset = frame * frame_height;

    for (int j = 0; j < frame_height; j++) {
        for (int i = 0; i < frame_width; i++) {
            uint32_t color = pixel_color(img, i, height_offset + j);
            uint8_t mix_alpha = color >> 24;
            if ((img->alpha_depth == 0) || img->alpha) {
                mix_alpha = pixel_alpha(img, i, height_offset + j);
            }
            if (alpha != 255) {
                mix_alpha = (mix_alpha * alpha) / 255;
            }

            color = hub75_alpha(mix_alpha, color);

            int draw_x, draw_y;
            switch (rotate) {
                case 0: // No rotation
                    draw_x = x + i;
                    draw_y = y + j;
                    break;
                case 1: // 90 degrees clockwise
                    draw_x = x + (frame_height - 1 - j);
                    draw_y = y + i;
                    break;
                case 2: // 180 degrees
                    draw_x = x + (frame_width - 1 - i);
                    draw_y = y + (frame_height - 1 - j);
                    break;
                case 3: // 270 degrees clockwise (90 counter-clockwise)
                    draw_x = x + j;
                    draw_y = y + (frame_width - 1 - i);
                    break;
                default:
                    draw_x = x + i;
                    draw_y = y + j;
                    break;
            }

            hub75_blend(draw_x, draw_y, color);
        }
    }
}
