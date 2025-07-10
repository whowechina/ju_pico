#include <stdlib.h>
#include <stdint.h>

#include "hub75.h"
#include "marker.h"

static void draw_pal8(const animation_t *ani, uint8_t x, uint8_t y, int frame) 
{
    int pixels = ani->image_size * ani->image_size;
    const uint8_t *img = ani->img8 + frame * pixels;

    for (int j = 0; j < ani->image_size; j++) {
        const uint8_t *row = img + j * ani->image_size;
        for (int i = 0; i < ani->image_size; i++) {
            uint8_t idx = row[i];
            uint32_t color = ani->palette[idx];
            hub75_pixel(x + i, y + j, color);
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
            hub75_pixel(x + i, y + j, color);
        }
    }
}

static void draw_24bit(const animation_t *ani, uint8_t x, uint8_t y, int frame)
{
    const uint32_t *img = ani->img32 + frame * ani->image_size * ani->image_size;
    for (int j = 0; j < ani->image_size; j++) {
        for (int i = 0; i < ani->image_size; i++) {
            uint32_t color = img[j * ani->image_size + i];
            hub75_pixel(x + i, y + j, color);
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
    } else if (ani->color_depth == 8) {
        draw_pal8(ani, x, y, frame);
    } else if (ani->color_depth == 24) {
        draw_24bit(ani, x, y, frame);
    }
}

static inline uint32_t progress_to_frame(const animation_t *ani, uint32_t progress)
{
    uint32_t total_frames = ani->frame_num;
    uint32_t frame = (progress * total_frames) / 1000;
    if (frame >= total_frames) {
        frame = total_frames - 1;
    }

    return frame;
}

void marker_draw(const marker_t *marker, marker_type type, uint8_t x, uint8_t y, uint32_t frame)
{
    if (!marker) {
        return;
    }

    const animation_t *ani = NULL;

    switch (type) {
        case MARKER_APPROACH:
            ani = &marker->approach;
            break;
        case MARKER_PERFECT:
            ani = &marker->perfect;
            break;
        case MARKER_GREAT:
            ani = &marker->great;
            break;
        case MARKER_GOOD:
            ani = &marker->good;
            break;
        case MARKER_POOR:
            ani = &marker->poor;
            break;
        case MARKER_MISS:
            ani = &marker->miss;
            break;
    }

    draw_frame(ani, x, y, frame);
}

void marker_draw_progress(const marker_t *marker, marker_type type, uint8_t x, uint8_t y, uint32_t progress)
{
    if (!marker) {
        return;
    }

    const animation_t *ani = NULL;

    switch (type) {
        case MARKER_APPROACH:
            ani = &marker->approach;
            break;
        case MARKER_PERFECT:
            ani = &marker->perfect;
            break;
        case MARKER_GREAT:
            ani = &marker->great;
            break;
        case MARKER_GOOD:
            ani = &marker->good;
            break;
        case MARKER_POOR:
            ani = &marker->poor;
            break;
        case MARKER_MISS:
            ani = &marker->miss;
            break;
    }

    draw_frame(ani, x, y, progress_to_frame(ani, progress));
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