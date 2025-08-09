#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include <stdbool.h>

/* LV Font is Simplified from LVGL (https://lvgl.io/) Font Structure */
typedef struct {
    uint32_t bitmap_index; /**< Start index of the bitmap. A font can be max 4 GB.*/
    uint32_t adv_w;        /**< Draw the next glyph after this width. 28.4 format (real_value * 16 is stored).*/
    uint16_t box_w;        /**< Width of the glyph's bounding box*/
    uint16_t box_h;        /**< Height of the glyph's bounding box*/
    int16_t ofs_x;         /**< x offset of the bounding box*/
    int16_t ofs_y;         /**< y offset of the bounding box. Measured from the top of the line*/
} lv_font_dsc_t;

typedef struct {
    uint8_t range_start;
    uint8_t range_length;
    uint8_t bit_per_pixel;
    uint16_t line_height;
    uint16_t base_line;
    const lv_font_dsc_t *dsc;
    const uint8_t *bitmap;
} lv_font_t;

unsigned int font_num();

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} alignment_t;

void font_spacing(int dx, int dy);

/* Color escape scheme:
  "\x01\xAA\xRR\xGG\xBB" set transparent color;
  "\x02\xRR\xGG\xBB" set solid color;
  "\x03\xRR\xGG\xBB" set color part only;
  "\x04" back to previous color;
  "\x05" reset to default color;
  "\x06\xDX\xDY" set spacing int8_t;
 */

#define ARGB(argb) "\x01" argb
#define SOLID(rgb) "\x02" rgb
#define RGB(rgb) "\x03" rgb
#define PREV_COLOR "\x04"
#define RESET_COLOR "\x05"
#define SPACING(dxdy) "\x06" dxdy

void font_draw_text(int x, int y, const char *text, int font,
                    uint32_t argb, alignment_t align);

// x_fp and y_fp are in 1/256 pixel units
void font_draw_text_smooth(int x_fp, int y_fp, const char *text, int font,
                           uint32_t argb, alignment_t align);

#endif