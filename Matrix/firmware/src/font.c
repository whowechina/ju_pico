#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "hub75.h"
#include "resource.h"
#include "font.h"

static int spacing_x = 1, spacing_y = 1;

unsigned int font_num()
{
    return font_count;
}

const lv_font_t *font_get(int index)
{
    if (index < 0 || index >= font_count) {
        return &font_lib[0];
    }
    return &font_lib[index];
}

static void font_draw_char(int x, int y, char c, const lv_font_t *font, uint32_t argb)
{
    if (c < font->range_start || c >= font->range_start + font->range_length) {
        return;
    }

    const lv_font_dsc_t *dsc = font->dsc + c - font->range_start;
    const uint8_t *bitmap = font->bitmap + dsc->bitmap_index;

    uint8_t bpp = font->bit_per_pixel;
    uint8_t mask = (1L << bpp) - 1;
    uint8_t off_y = font->line_height - font->base_line - dsc->box_h - dsc->ofs_y;

    for (int i = 0; i < dsc->box_h; i++) {
        int dot_y = y + off_y + i;
        if ((dot_y < 0) || (dot_y > PANEL_HEIGHT)) {
            continue;
        }
        for (int j = 0; j < dsc->box_w; j++) {
            int dot_x = x + dsc->ofs_x + j;
            if ((dot_x < 0) || (dot_x > PANEL_WIDTH)) {
                break;
            }
            uint16_t bits = (i * dsc->box_w + j) * bpp;
            uint8_t mix = (bitmap[bits / 8] >> ((8 - bpp) - (bits % 8))) & mask;
            mix = ((mix * 255) / mask) * (argb >> 24) / 255;
            hub75_blend(dot_x, dot_y, hub75_alpha(mix, argb));
        }
    }
}


void font_spacing(int dx, int dy)
{
    spacing_x = dx;
    spacing_y = dy;
}

static uint16_t textline_width(const char *text, const lv_font_t *font, int spacing_x)
{
    int dx = spacing_x;
    uint16_t width = 0;
    bool first_char = true;

    for (; *text && (*text != '\n'); text++) {
        if (*text == '\x01') {
            text += 4;
            continue;
        } else if ((*text == '\x02') || (*text == '\x03')) {
            text += 3;
            continue;
        } else if ((*text == '\x04')  || (*text == '\x05')) {
            text += 1;
            continue;
        } else if (*text == '\x06') {
            text += 2;
            dx = (int8_t)text[1];
            continue;
        }
    
        if (*text - font->range_start < font->range_length) {
            width += (font->dsc[*text - font->range_start].adv_w >> 4);
            if (!first_char) {
                width += dx;
            }
            first_char = false;
        }
    }
    return width;
}

void font_draw_text(int x, int y, const char *text, int font,
                    uint32_t argb, alignment_t align)
{
    const lv_font_t *f = font_get(font);

    int dx = spacing_x;
    int dy = spacing_y;

    uint32_t old_color = argb;
    uint32_t curr_color = argb;

    bool newline = true;
    int pos_x = 0;
    int pos_y = 0;

    for (; *text; text++) {
        if (*text == '\x01') { // set transparent color argb
            old_color = curr_color;
            curr_color = hub75_argb(text[1], text[2], text[3], text[4]);
            text += 4;
            continue;
        } else if (*text == '\x02') { // set solid color rgb
            old_color = curr_color;
            curr_color = hub75_argb(0xff, text[1], text[2], text[3]);
            text += 3;
            continue;
        } else if (*text == '\x03') { // set color part only rgb
            old_color = curr_color;
            curr_color &= 0xff000000;
            curr_color |= hub75_rgb(text[1], text[2], text[3]);
            text += 3;
            continue;
        } else if (*text == '\x04') { // back to previous color
            uint32_t tmp = curr_color;
            curr_color = old_color;
            old_color = tmp;
            continue;
        } else if (*text == '\x05') { // reset to default color
            old_color = curr_color;
            curr_color = argb;
            continue;
        } else if (*text == '\x06') { // set spacing
            dx = (int8_t)text[1];
            dy = (int8_t)text[2];
            text += 2;
            continue;
        } else if (*text == '\n') { // line wrap
            newline = true;
            pos_y += f->line_height + dy;
            continue;
        }

        if (newline) {
            int width = textline_width(text, f, dx);
            if (align == ALIGN_CENTER) {
                pos_x -= width / 2;
            } else if (align == ALIGN_RIGHT) {
                pos_x -= width;
            }
            newline = false;
        }

        font_draw_char(x + pos_x, y + pos_y, *text, f, curr_color);
        pos_x += (f->dsc[*text - f->range_start].adv_w >> 4) + dx;
    }
}
