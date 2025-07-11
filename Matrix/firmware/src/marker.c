#include <stdlib.h>
#include <stdint.h>

#include "hub75.h"
#include "marker.h"

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

static inline int get_alpha(const animation_t *ani, uint8_t x, uint8_t y, int frame)
{
    int pixels = ani->image_size * ani->image_size;
    int start_pixel = frame * pixels + y * ani->image_size + x;
    int pixel_per_byte = 8 / ani->alpha_depth;
    int byte = start_pixel / pixel_per_byte;
    int section = start_pixel % pixel_per_byte;
    return extract_alpha(ani->alpha_depth, ani->alpha[byte], section);
}

static void draw_pal8(const animation_t *ani, uint8_t x, uint8_t y, int frame) 
{
    int pixels = ani->image_size * ani->image_size;
    const uint8_t *img = ani->img8 + frame * pixels;

    for (int j = 0; j < ani->image_size; j++) {
        const uint8_t *row = img + j * ani->image_size;
        for (int i = 0; i < ani->image_size; i++) {
            uint8_t idx = row[i];
            uint32_t color = ani->palette[idx];
            uint8_t alpha = get_alpha(ani, i, j, frame);
            hub75_blend(x + i, y + j, (alpha << 24) | color);
        }
    }
}

/* 5 bit pal, 3 bit alpha encoded in pixels */
static void draw_pal5(const animation_t *ani, uint8_t x, uint8_t y, int frame) 
{
    int pixels = ani->image_size * ani->image_size;
    const uint8_t *img = ani->img8 + frame * pixels;

    for (int j = 0; j < ani->image_size; j++) {
        const uint8_t *row = img + j * ani->image_size;
        for (int i = 0; i < ani->image_size; i++) {
            uint8_t idx = row[i];
            uint32_t color = ani->palette[idx & 0x1f];
            uint8_t alpha = (idx >> 5) * 255 / 7;
            hub75_blend(x + i, y + j, (alpha << 24) | color);
        }
    }
}

static void draw_pal4(const animation_t *ani, uint8_t x, uint8_t y, int frame)
{
    int img_size = ani->image_size;
    int pixels_per_frame = img_size * img_size;
    
    for (int j = 0; j < img_size; j++) {
        for (int i = 0; i < img_size; i++) {
            int pixel_index = frame * pixels_per_frame + j * img_size + i;
            int byte_index = pixel_index / 2;
            uint8_t byte = ani->img8[byte_index];
            uint8_t idx;
            
            if (pixel_index % 2 == 0) {
                idx = (byte >> 4) & 0x0f;
            } else {
                idx = byte & 0x0f;
            }
            
            uint32_t color = ani->palette[idx];
            uint8_t alpha = get_alpha(ani, i, j, frame);
            hub75_blend(x + i, y + j, (alpha << 24) | color);
        }
    }
}

static void draw_24bit(const animation_t *ani, uint8_t x, uint8_t y, int frame)
{
    const uint32_t *img = ani->img32 + frame * ani->image_size * ani->image_size;
    for (int j = 0; j < ani->image_size; j++) {
        for (int i = 0; i < ani->image_size; i++) {
            uint32_t color = img[j * ani->image_size + i];
            hub75_blend(x + i, y + j, color);
        }
    }
}

static void draw_frame(const animation_t *ani, uint8_t x, uint8_t y, uint32_t frame)
{
    if (!ani) {
        return;
    }

    if (ani->color_depth == 4) {
        draw_pal4(ani, x, y, frame);
    } else if (ani->color_depth == 5) {
        draw_pal5(ani, x, y, frame);
    } else if (ani->color_depth == 8) {
        draw_pal8(ani, x, y, frame);
    } else if (ani->color_depth == 24) {
        draw_24bit(ani, x, y, frame);
    }
}

static inline int time_to_frame(int fps, int frame_num, uint32_t time_ms)
{
    uint32_t frame_time_us = 1000000 / fps;
    uint32_t frame = time_ms * 1000 / frame_time_us;
    return frame >= frame_num ? - 1 : frame;
}

bool marker_is_end(const marker_t *marker, marker_type type, uint32_t time_ms)
{
    if (!marker || type < 0 || type >= 6) {
        return true;
    }
    uint32_t frame_time_us = 1000000 / marker->fps;
    return time_ms * 1000 > marker->types[type].frame_num * frame_time_us;
}

void marker_draw(const marker_t *marker, marker_type type, uint8_t x, uint8_t y, uint32_t time_ms)
{
    if ((!marker) || (marker->fps == 0)) {
        return;
    }

    const animation_t *ani = NULL;

    ani = &marker->types[type];

    draw_frame(ani, x, y, time_to_frame(marker->fps, ani->frame_num, time_ms));
}

void marker_clear(uint8_t x, uint8_t y, uint32_t color)
{
    int col = x * 17;
    int row = y * 17;
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            hub75_pixel(col + i, row + j, color);
        }
    }
}