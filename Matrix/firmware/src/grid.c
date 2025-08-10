#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "pico/time.h"

#include "config.h"
#include "marker.h"
#include "hub75.h"
#include "grid.h"
#include "font.h"

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
    bool attached;
    uint64_t start;
    uint32_t dir : 2; // 0: down, 1: up, 2: right, 3: left
    uint32_t len : 2;
    uint32_t span : 24;
    uint32_t remain;
    bool moving;
} trail_t;

typedef struct {
    bool started;
    uint8_t marker;
    uint8_t mode;
    uint64_t schedule;
    uint64_t start;
    trail_t trail;
} grid_cell_t;

static struct {
    uint32_t preview;
    grid_cell_t grid[4][4];
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
    cell->started = true;
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

void grid_schedule(int col, int row, bool override)
{
    if (out_of_bound(col, row)) {
        return;
    }

    grid_cell_t *cell = &grid_ctx.grid[col][row];

    if (!override) {
        if (cell->started || (cell->schedule > 0)) {
            return;
        }
    }

    cell->marker = matrix_cfg->game.marker;
    cell->mode = MARKER_APPROACH;

    cell->trail.attached = false;
    cell->trail.moving = false;
    cell->started = false;
    cell->schedule = time_us_64() + matrix_cfg->game.start_delay_ms * 1000;
}

void grid_judge(int col, int row, marker_mode_t mode)
{
    if (out_of_bound(col, row)) {
        return;
    }

    if (!grid_is_started(col, row)) {
        return;
    }

    grid_cell_t *cell = &grid_ctx.grid[col][row];
    cell->mode = mode;
    cell->start = time_us_64();

    if (cell->trail.attached) {
        if (!cell->trail.moving) {
            cell->trail.moving = true;
            cell->trail.start = time_us_64();
        }
    }
}

void grid_abort(int col, int row)
{
    if (out_of_bound(col, row)) {
        return;
    }
    grid_ctx.grid[col][row].started = false;
    grid_ctx.grid[col][row].schedule = 0;
    grid_ctx.grid[col][row].trail.attached = false;
}

bool grid_is_active(int col, int row)
{
    if (out_of_bound(col, row)) {
        return false;
    }
    
    grid_cell_t *cell = &grid_ctx.grid[col][row];
    return cell->started || (cell->schedule > 0);
}

bool grid_is_started(int col, int row)
{
    if (out_of_bound(col, row)) {
        return false;
    }
    
    grid_cell_t *cell = &grid_ctx.grid[col][row];
    return cell->started;
}

bool grid_is_moving(int col, int row)
{
    if (out_of_bound(col, row)) {
        return false;
    }

    grid_cell_t *cell = &grid_ctx.grid[col][row];
    return (cell->started && cell->trail.attached && cell->trail.moving);
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
        cell->started = true;
        return;
    }

    if (!cell->started) {
        return;
    }

    trail_t *trail = &cell->trail;
    if (grid_is_moving(col, row)) {
        int elapsed = (update_time - trail->start) / 3333;
        int remain = trail->span - elapsed;
        if (remain >= 0) {
            trail->remain = remain;
        } else if (remain > -15) {
            trail->remain = 0;
        } else {
            trail->moving = false;
            trail->attached = false;
        }
    }

    uint64_t elapsed = update_time - cell->start;

    if (!marker_is_end(cell->marker, cell->mode, elapsed)) {
        return;
    }

    if (cell->trail.attached && cell->trail.moving) {
        return;
    }

    cell->started = false;
    cell->trail.attached = false;

    if (grid_ctx.on_finish) {
        grid_ctx.on_finish(col, row, cell->mode);
    }
}

void grid_update()
{
    update_time = time_us_64();

    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            grid_cell_t *cell = &grid_ctx.grid[col][row];
            update_cell(cell, col, row);
        }
    }
}

static int triangle_wave(int low, int high, int elapsed, int cycle_ms)
{
    int amp = high - low;
    int pos = (elapsed / 1000 % cycle_ms) * (amp * 2) / cycle_ms;
    if (pos <= amp) {
        return low + pos;
    } else {
        return high - (pos - amp);
    }
}

static void trail_draw(int x, int y, const trail_t *trail)
{
    if (!trail->attached) {
        return;
    }

    uint64_t now = time_us_64();
    uint64_t elapsed = now - trail->start;

    int frame = 0;
    int alpha = 255;
    if (trail->moving) {
        frame = elapsed / 33333;
        alpha = triangle_wave(64, 255, elapsed, 533);
    }

    marker_draw_socket(x, y, 0, trail->moving, trail->dir);

    int distance = (trail->remain * trail->len * GRID_PITCH) / trail->span;

    if (distance > GRID_PITCH) {
        marker_draw_stem(x, y, frame, GRID_PITCH, trail->dir, alpha);
    }

    if (distance > GRID_PITCH * 2)
    {
        marker_draw_stem(x, y, frame, GRID_PITCH * 2, trail->dir, alpha);
    }

    if (frame < marker_arrow_grow_frames()) {
        marker_draw_arrow_grow(x, y, frame, trail->len * GRID_PITCH, trail->dir, alpha);
    } else {
        marker_draw_arrow(x, y, distance, trail->dir, alpha);
    }

    if (trail->moving) {
        marker_draw_glow(x, y, frame);
    }
}

void grid_render()
{
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            grid_cell_t *cell = &grid_ctx.grid[col][row];
            if (!cell->started) {
                continue;
            }

            int x = col * GRID_PITCH + GRID_OFFSET_X;
            int y = row * GRID_PITCH + GRID_OFFSET_Y;

            if (cell->trail.attached) {
                trail_draw(x, y, &cell->trail);
            }

            uint64_t elapsed = update_time - cell->start;
            marker_draw(x, y, cell->marker, cell->mode, elapsed);
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

void grid_attach_trail(int col, int row, int dir, int len, uint32_t span, bool override)
{
    if (out_of_bound(col, row)) {
        return;
    }

    if (span == 0) {
        return;
    }

    if (!grid_is_active(col, row)) {
        return;
    }

    grid_cell_t *cell = &grid_ctx.grid[col][row];
    trail_t *trail = &cell->trail;

    if ((!override) && (trail->attached)) {
        return;
    }

    const char *dirs[] = {"down", "up", "right", "left"};

    printf("Trail: %d%d %s L:%d\n", col, row, dirs[dir], len);

    trail->attached = true;
    trail->moving = false;
    trail->dir = dir;
    trail->len = len;
    trail->span = span;
    trail->remain = span;
}

void grid_update_trail(int col, int row, float ratio)
{
    if (out_of_bound(col, row)) {
        return;
    }

    grid_cell_t *cell = &grid_ctx.grid[col][row];
    trail_t *trail = &cell->trail;

    if (!trail->attached) {
        return;
    }

    if (!trail->moving) {
        trail->moving = true;
        trail->start = time_us_64();
        int overtime = ratio * 3333 * trail->span;
        trail->start -= overtime / trail->len;
    }
}

void grid_end_trail(int col, int row)
{
    if (out_of_bound(col, row)) {
        return;
    }

    trail_t *trail = &grid_ctx.grid[col][row].trail;
    trail->attached = false;
}

void grid_listen(grid_marker_finish_cb on_finish)
{
    grid_ctx.on_finish = on_finish;
}
