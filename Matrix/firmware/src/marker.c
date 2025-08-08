/*
 * Marker Animation Plotter
 * WHowe <github.com/whowechina>
 */

#include <stdlib.h>
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

    int frame_num = res->modes[mode].frame_num;
    if (mode == MARKER_APPROACH) {
        frame_num += res->modes[MARKER_MISS].frame_num;
    }
    uint32_t frame_time_us = 1000000 / res->fps;

    return elapsed > frame_num * frame_time_us;
}

void marker_draw(int x, int y, int marker, marker_mode_t mode, uint32_t elapsed)
{
    if (!marker_mode_is_valid(marker, mode)) {
        return;
    }

    const marker_res_t *res = &marker_lib[marker];
    const animation_t *ani = &res->modes[mode];

    int frame = calc_frame(res->fps, elapsed);

    int frame_num = ani->frame_num;
    if (mode == MARKER_APPROACH) {
        frame_num += res->modes[MARKER_MISS].frame_num;
        if (frame >= res->modes[MARKER_APPROACH].frame_num) {
            frame -= res->modes[MARKER_APPROACH].frame_num;
            ani = &res->modes[MARKER_MISS];
        }
    }

    if (frame >= ani->frame_num) {
        frame = ani->frame_num - 1;
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
    image_draw(x, y, &img,frame);
}

void marker_clear(int x, int y, uint32_t color)
{
    for (int i = 0; i < marker_lib[0].modes[0].image_size; i++) {
        for (int j = 0; j < marker_lib[0].modes[0].image_size; j++) {
            hub75_blend(x + i, y + j, color);
        }
    }
}
