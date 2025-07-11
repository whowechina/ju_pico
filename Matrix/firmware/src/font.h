#ifndef FONT_H
#define FONT_H

#include <stdint.h>

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t first_char;
    uint8_t char_count;
    uint8_t bit_per_pixel; /* 1, 2, 4, or 8 */
    const uint8_t *data;
} font_t;

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} alignment_t;

void font_draw_string(const font_t *font, uint8_t x, uint8_t y,
                      const char *str, uint32_t color, alignment_t align, int pitch);
#endif