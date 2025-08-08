/*
 * Marker Animation Plotter
 * WHowe <github.com/whowechina>
 */

 #ifndef MARKER_H
#define MARKER_H

#include <stdint.h>
#include <stdbool.h>

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
        const uint8_t *image;
    };
} animation_t;

typedef struct {
    uint8_t fps;
    animation_t modes[6];
} marker_res_t;

typedef enum {
    MARKER_APPROACH = 0,
    MARKER_PERFECT,
    MARKER_GREAT,
    MARKER_GOOD,
    MARKER_POOR,
    MARKER_MISS,
    MARKER_MODE_NUM,
} marker_mode_t;

unsigned int marker_num();
bool marker_is_end(int marker, marker_mode_t mode, uint32_t elapsed);
void marker_draw(int x, int y, int marker, marker_mode_t mode, uint32_t elapsed);
void marker_clear(int x, int y, uint32_t color);

#endif
