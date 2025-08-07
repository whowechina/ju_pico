#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "pico/time.h"

#include "config.h"
#include "marker.h"
#include "hub75.h"
#include "grid.h"

#ifndef GRID_OFFSET_X
#define GRID_OFFSET_X 0
#endif
#ifndef GRID_OFFSET_Y
#define GRID_OFFSET_Y 0
#endif

#define GRID_PITCH (GRID_SIZE + GRID_GAP)

unsigned int grid_pitch()
{
    return GRID_PITCH;
}

typedef struct {
    bool active;
    uint8_t marker;
    uint8_t mode;
    uint64_t schedule;
    uint64_t start;
} grid_cell_t;

typedef struct {
    bool active;
    bool pressed;
    uint64_t schedule;
    uint64_t start;
    uint32_t dir : 2; // 0: down, 1: up, 2: right, 3: left
    uint32_t len : 2;
    uint32_t span : 24;
    float ratio;
} grid_trail_t;

static struct {
    uint32_t preview;
    grid_cell_t grid[4][4];
    grid_trail_t trail[4][4];
    grid_marker_finish_cb on_finish;
} grid_ctx = { 0 };

static inline bool out_of_bound(int col, int row)
{
    return (col < 0) || (col >= 4) || (row < 0) || (row >= 4);
}

void grid_test(int col, int row, int marker)
{
    if (out_of_bound(col, row)) {
        return;
    }

    grid_cell_t *cell = &grid_ctx.grid[col][row];

    cell->marker = marker;
    cell->mode = MARKER_APPROACH;
    cell->schedule = 0;
    cell->start = time_us_64();
    cell->active = true;
}

void grid_preview(int col, int row)
{
    if (out_of_bound(col, row)) {
        return;
    }

    grid_ctx.preview |= (1 << (row * 4 + col));
}

void grid_preview_reset()
{
    grid_ctx.preview = 0;
}

void grid_start(int col, int row, bool override)
{
    if (out_of_bound(col, row)) {
        return;
    }

    grid_cell_t *cell = &grid_ctx.grid[col][row];

    if (!override) {
        if (cell->active || (cell->schedule > 0)) {
            return;
        }
    }

    cell->marker = matrix_cfg->game.marker;
    cell->mode = MARKER_APPROACH;

    cell->schedule = time_us_64() + matrix_cfg->game.start_delay_ms * 1000;
}

void grid_judge(int col, int row, marker_mode_t mode)
{
    if (out_of_bound(col, row)) {
        return;
    }
    grid_cell_t *cell = &grid_ctx.grid[col][row];
    cell->mode = mode;

    cell->start = time_us_64();
    cell->active = true;

    if (grid_ctx.trail[col][row].active) {
        printf("%4ld-Judge: %d%d %d\n", time_us_32() / 1000 % 10000, col, row, mode);
    }
}

void grid_abort(int col, int row)
{
    if (out_of_bound(col, row)) {
        return;
    }
    grid_ctx.grid[col][row].active = false;
    grid_ctx.grid[col][row].schedule = 0;
}

bool grid_is_active(int col, int row)
{
    if (out_of_bound(col, row)) {
        return false;
    }
    return grid_ctx.grid[col][row].active || (grid_ctx.grid[col][row].schedule > 0);
}

int grid_last_marker(int col, int row)
{
    if (out_of_bound(col, row)) {
        return -1;
    }
    return grid_ctx.grid[col][row].marker;
}

int grid_last_mode(int col, int row)
{
    if (out_of_bound(col, row)) {
        return -1;
    }
    return grid_ctx.grid[col][row].mode;
}

static uint64_t update_time;

static void update_cell(grid_cell_t *cell, int col, int row)
{
    if ((cell->schedule > 0) && (update_time >= cell->schedule)) {
        cell->schedule = 0;
        cell->start = update_time;
        cell->active = true;
        return;
    }

    if (!cell->active) {
        return;
    }

    uint64_t elapsed = update_time - cell->start;
    if (!marker_is_end(cell->marker, cell->mode, elapsed)) {
        return;
    }

    cell->active = false;

    if (grid_ctx.on_finish) {
        grid_ctx.on_finish(col, row, cell->mode);
    }
}

static void update_trail(grid_trail_t *trail, int col, int row)
{
    if (trail->schedule > 0) {
        if (update_time >= trail->schedule) {
            trail->schedule = 0;
            trail->start = update_time;
            trail->active = true;
        } else {
            return;
        }
    }

    if (!trail->active) {
        return;
    }
}

void grid_update()
{
    update_time = time_us_64();

    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            grid_cell_t *cell = &grid_ctx.grid[col][row];
            update_cell(cell, col, row);
            grid_trail_t *trail = &grid_ctx.trail[col][row];
            update_trail(trail, col, row);
        }
    }
}

static void trail_draw(int col, int row, uint32_t dir, uint32_t len, float ratio)
{
    int sx = col * GRID_PITCH + GRID_OFFSET_X + GRID_SIZE / 2;
    int sy = row * GRID_PITCH + GRID_OFFSET_Y + GRID_SIZE / 2;
    int dx = sx;
    int dy = sy;

    int real_len = (int)(ratio * GRID_PITCH);
    switch (dir) {
        case 0: // down
            sy -= real_len;
            break;
        case 1: // up
            dy += real_len;
            break;
        case 2: // right
            sx -= real_len;
            break;
        case 3: // left
            dx += real_len;
            break;
    }

    for (int i = sx; i <= dx; i++) {
        for (int j = sy; j <= dy; j++) {
            hub75_blend(i, j, hub75_argb(0xc0, 200, 200, 200));
        }
    }
}


void grid_render()
{
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            grid_cell_t *cell = &grid_ctx.grid[col][row];
            uint64_t elapsed = update_time - cell->start;
            if (cell->active) {
                int x = col * GRID_PITCH + GRID_OFFSET_X;
                int y = row * GRID_PITCH + GRID_OFFSET_Y;
                marker_draw(x, y, cell->marker, cell->mode, elapsed);
            }

            grid_trail_t *trail = &grid_ctx.trail[col][row];
            if (trail->active) {
                trail_draw(col, row, trail->dir, trail->len, trail->ratio);
            }
        }
    }
}

static void draw_preview(int x, int y, uint8_t alpha)
{
    uint8_t hue = time_us_32() >> 12;
    uint32_t color = hub75_hsv2rgb(hue, 220, 200);
    marker_clear(x, y, hub75_alpha(alpha, color));
}

void grid_render_preview(uint8_t alpha)
{
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            if (grid_ctx.preview & (1 << (row * 4 + col))) {
                int x = col * GRID_PITCH + GRID_OFFSET_X;
                int y = row * GRID_PITCH + GRID_OFFSET_Y;
                draw_preview(x, y, alpha);
            }
        }
    }
}

void grid_start_trail(int col, int row, int dir, int len, uint32_t span, bool override)
{
    if (out_of_bound(col, row)) {
        return;
    }

    grid_trail_t *trail = &grid_ctx.trail[col][row];
    
    if (!override) {
        if (trail->active || (trail->schedule > 0)) {
            return;
        }
    }

    printf("Trail: %d%d %d %d\n", col, row, dir, len);

    trail->pressed = false;
    trail->dir = dir;
    trail->len = len;
    trail->span = span;
    trail->ratio = len;
    trail->schedule = time_us_64() + matrix_cfg->game.start_delay_ms * 1000;
}

void grid_trail_update(int col, int row, float ratio)
{
    if (out_of_bound(col, row)) {
        return;
    }

    grid_trail_t *trail = &grid_ctx.trail[col][row];
    if (!trail->active) {
        return;
    }

    trail->ratio = ratio;
}

void grid_end_trail(int col, int row)
{
    if (out_of_bound(col, row)) {
        return;
    }

    grid_trail_t *trail = &grid_ctx.trail[col][row];
    trail->active = false;
    trail->schedule = 0;
}

void grid_listen(grid_marker_finish_cb on_finish)
{
    grid_ctx.on_finish = on_finish;
}
