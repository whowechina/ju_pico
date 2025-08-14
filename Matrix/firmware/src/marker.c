/*
 * Marker Animation Plotter
 * WHowe <github.com/whowechina>
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "hub75.h"
#include "marker.h"
#include "image.h"
#include "resource.h"

static inline int calc_frame(int fps, uint32_t elapsed)
{
    uint32_t frame_time_us = 1000000 / fps;
    uint32_t frame = elapsed / frame_time_us;
    return frame;
}

unsigned int marker_num()
{
    return marker_count;
}

static inline bool marker_mode_is_valid(int marker, marker_mode_t mode)
{
    if ((marker < 0) || (marker >= marker_count)) {
        return false;
    }
    if ((mode < 0) || (mode >= MARKER_MODE_NUM)) {
        return false;
    }
    return true;
}

bool marker_is_end(int marker, marker_mode_t mode, uint32_t elapsed)
{
    if (!marker_mode_is_valid(marker, mode)) {
        return true;
    }

    const marker_res_t *res = &marker_lib[marker];

    int total_frame = res->modes[mode].frame_num;
    if (mode == MARKER_APPROACH) {
        total_frame += res->modes[MARKER_MISS].frame_num;
    }

    int frame = calc_frame(res->fps, elapsed);

    return frame >= total_frame;
}

void marker_draw(int x, int y, int marker, marker_mode_t mode, uint32_t elapsed)
{
    if (!marker_mode_is_valid(marker, mode)) {
        return;
    }

    const marker_res_t *res = &marker_lib[marker];
    const animation_t *ani = &res->modes[mode];

    int frame = calc_frame(res->fps, elapsed);

    int total_frame = ani->frame_num;
    if (mode == MARKER_APPROACH) {
        total_frame += res->modes[MARKER_MISS].frame_num;
        if (frame >= res->modes[mode].frame_num) {
            frame -= res->modes[mode].frame_num;
            ani = &res->modes[MARKER_MISS];
        }
    }

    if (frame >= ani->frame_num) {
        return;
    }

    const image_t img = {
        .color_depth = ani->color_depth,
        .alpha_depth = ani->alpha_depth,
        .width = ani->image_size,
        .height = ani->image_size * ani->frame_num,
        .frame_num = ani->frame_num,
        .palette = ani->palette,
        .alpha = ani->alpha,
        .image = ani->image,
    };

    image_draw(x, y, &img, frame, 255, 0);
}

void marker_clear(int x, int y, uint32_t color)
{
    for (int i = 0; i < marker_lib[0].modes[0].image_size; i++) {
        for (int j = 0; j < marker_lib[0].modes[0].image_size; j++) {
            hub75_blend(x + i, y + j, color);
        }
    }
}

// "down", "up", "right", "left"
const uint8_t dir_rotate[4] = {0, 2, 3, 1};

static void draw_frame(int x, int y, const animation_t *ani, int frame, uint8_t alpha, uint8_t dir)
{
    if (ani->frame_num == 0) {
        return;
    }

    const image_t img = {
        .color_depth = ani->color_depth,
        .alpha_depth = ani->alpha_depth,
        .width = ani->image_size,
        .height = ani->image_size * ani->frame_num,
        .frame_num = ani->frame_num,
        .palette = ani->palette,
        .alpha = ani->alpha,
        .image = ani->image,
    };

    image_draw(x, y, &img, frame % ani->frame_num, alpha, dir_rotate[dir]);
}


void marker_draw_glow(int x, int y, int frame)
{
    draw_frame(x, y, &trail_res.marker_glow, frame, 128, 0);
}

void marker_draw_socket(int x, int y, int frame, bool active, int dir)
{
    if (active) {
        draw_frame(x, y, &trail_res.marker_on, frame, 128, dir);
    } else {
        draw_frame(x, y, &trail_res.marker_off, frame, 128, dir);
    }
}

static int x_offset(int x, int distance, int dir)
{
    if (dir == 2) {
        return x - distance;
    }
    if (dir == 3) {
        return x + distance;
    }
    return x;
}

static int y_offset(int y, int distance, int dir)
{
    if (dir == 0) {
        return y - distance;
    }
    if (dir == 1) {
        return y + distance;
    }
    return y;
}

void marker_draw_arrow(int x, int y, int distance, int dir, int alpha)
{
    int draw_x = x_offset(x, distance, dir);
    int draw_y = y_offset(y, distance, dir);
    draw_frame(draw_x, draw_y, &trail_res.arrow, 0, alpha, dir);
}

void marker_draw_arrow_grow(int x, int y, int frame, int distance, int dir, int alpha)
{
    int draw_x = x_offset(x, distance, dir);
    int draw_y = y_offset(y, distance, dir);
    draw_frame(draw_x, draw_y, &trail_res.arrow_grow, frame, alpha, dir);
}

unsigned int marker_arrow_grow_frames()
{
    return 4; // trail_res.arrow_grow.frame_num;
}

void marker_draw_stem(int x, int y, int frame, int distance, int dir, int alpha)
{
    int draw_x = x_offset(x, distance, dir);
    int draw_y = y_offset(y, distance, dir);
    draw_frame(draw_x, draw_y, &trail_res.stem, frame, alpha, dir);
}
