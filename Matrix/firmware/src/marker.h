#ifndef MARKER_H
#define MARKER_H

#include <stdint.h>

typedef struct {
    uint8_t color_depth; /* 4, 8, or 24 */
    uint8_t image_size; /* 13 means 13*13 pixels */
    uint16_t frame_num;
    const uint32_t *palette;
    union {
        const uint32_t *img32;
        const uint8_t *img8;
    };
} animation_t;

typedef struct {
    animation_t approach;
    animation_t perfect;
    animation_t great;
    animation_t good;
    animation_t poor;
    animation_t miss;
} marker_t;

typedef enum {
    MARKER_APPROACH,
    MARKER_PERFECT,
    MARKER_GREAT,
    MARKER_GOOD,
    MARKER_POOR,
    MARKER_MISS
} marker_type;

void marker_draw(const marker_t *marker, marker_type type, uint8_t x, uint8_t y, uint32_t frame);
void marker_draw_progress(const marker_t *marker, marker_type type, uint8_t x, uint8_t y, uint32_t progress);
void marker_clear(uint8_t x, uint8_t y, uint32_t color);

#endif
