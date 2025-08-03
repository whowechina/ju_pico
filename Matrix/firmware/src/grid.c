#include <stdlib.h>
#include <stdint.h>
#include "pico/time.h"

#include "config.h"
#include "marker.h"
#include "hub75.h"
#include "grid.h"

typedef struct {
    bool active;
    uint8_t marker;
    uint8_t mode;
    uint64_t schedule;
    uint64_t start;
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

    if (!override) {
        if (grid_ctx.grid[col][row].active || (grid_ctx.grid[col][row].schedule > 0)) {
            return;
        }
    }

    grid_cell_t *cell = &grid_ctx.grid[col][row];

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
}

void grid_abort(int col, int row)
{
    if (out_of_bound(col, row)) {
        return;
    }
    grid_ctx.grid[col][row].active = false;
}

bool grid_is_active(int col, int row)
{
    if (out_of_bound(col, row)) {
        return false;
    }
    return grid_ctx.grid[col][row].active;
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

void grid_update()
{
    update_time = time_us_64();

    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            grid_cell_t *cell = &grid_ctx.grid[col][row];
            if ((cell->schedule > 0) && (update_time >= cell->schedule)) {
                cell->schedule = 0;
                cell->start = update_time;
                cell->active = true;
                continue;
            }

            if (!cell->active) {
                continue;
            }

            uint64_t elapsed = update_time - cell->start;
            if (!marker_is_end(cell->marker, cell->mode, elapsed)) {
                continue;
            }
            cell->active = false;
            if (grid_ctx.on_finish) {
                grid_ctx.on_finish(col, row, cell->mode);
            }
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
                marker_draw(col * 17, row * 17, cell->marker, cell->mode, elapsed);
            }
        }
    }
}

void grid_render_preview()
{
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            if (grid_ctx.preview & (1 << (row * 4 + col))) {
                int x = col * 17;
                int y = row * 17;
                uint8_t hue = (time_us_32() >> 13);
                marker_clear(x, y, hub75_hsv2rgb(hue, 255, 100));
            }
        }
    }
}

void grid_listen(grid_marker_finish_cb on_finish)
{
    grid_ctx.on_finish = on_finish;
}
