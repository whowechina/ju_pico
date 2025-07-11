#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "hub75.h"
#include "font.h"

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

/* Font grayscale is actually the alpha channel */
static void draw_char(const font_t *font, uint8_t x, uint8_t y, char c, uint32_t color)
{
    if ((c < font->first_char) || (c >= font->first_char + font->char_count)) {
        return;
    }

    color &= 0x00ffffff;

    int width = font->width;
    int height = font->height;
    int pixel_per_byte = 8 / font->bit_per_pixel;
    int start_pixel = (c - font->first_char) * width * height;

    for (int j = 0; j < height; j++) {
        int row_start = start_pixel + j * width;
        for (int i = 0; i < width; i++) {
            int byte = (row_start + i) / pixel_per_byte;
            int section = (row_start + i) % pixel_per_byte;
            uint8_t data = font->data[byte];

            uint8_t alpha = extract_alpha(font->bit_per_pixel, data, section);
            hub75_blend(x + i, y + j, (alpha << 24) | color);
        }
    }
}


static inline int string_width(const font_t *font, const char *str, int pitch)
{
    int len = strlen(str);
    if (len == 0) {
        return 0;
    }
    return font->width * len + pitch * (len - 1);
}

void font_draw_string(const font_t *font, uint8_t x, uint8_t y,
                      const char *str, uint32_t color, alignment_t align, int pitch)
{
    if (!font || !str) {
        return;
    }
    
    int width = string_width(font, str, pitch);
    int start_x = x;
    
    switch (align) {
        case ALIGN_CENTER:
            start_x = x - (width + 1) / 2;
            break;
        case ALIGN_RIGHT:
            start_x = x - width;
            break;
        default:
            break;
    }
    
    for (const char *p = str; *p; p++) {
        draw_char(font, start_x, y, *p, color);
        start_x += font->width + pitch;
    }
}
