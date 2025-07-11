#ifndef MARKER_H
#define MARKER_H

#include <stdint.h>

typedef struct {
    uint8_t color_depth; /* 4, 8, or 24 */
    uint8_t alpha_depth;
    uint8_t image_size; /* 13 means 13*13 pixels */
    uint8_t frame_num;
    const uint32_t *palette;
    const uint8_t *alpha;
    union {
        const uint32_t *img32;
        const uint8_t *img8;
    };
} animation_t;

typedef struct {
    uint8_t fps;
    animation_t types[6];
} marker_t;

typedef enum {
    MARKER_APPROACH = 0,
    MARKER_PERFECT,
    MARKER_GREAT,
    MARKER_GOOD,
    MARKER_POOR,
    MARKER_MISS
} marker_type;

bool marker_is_end(const marker_t *marker, marker_type type, uint32_t time_ms);
void marker_draw(const marker_t *marker, marker_type type, uint8_t x, uint8_t y, uint32_t time_ms);
void marker_clear(uint8_t x, uint8_t y, uint32_t color);

#endif
