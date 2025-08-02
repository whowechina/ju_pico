#include <stdlib.h>
#include <stdint.h>
#include "pico/time.h"

#include "config.h"
#include "marker.h"
#include "grid.h"

typedef struct {
    bool active;
    uint8_t marker;
    uint8_t mode;
    uint64_t schedule;
    uint64_t start;
} grid_cell_t;

static struct {
    grid_cell_t grid[4][4];
} ctx = { 0 };

static inline bool out_of_bound(int col, int row)
{
    return (col < 0) || (col >= 4) || (row < 0) || (row >= 4);
}

void grid_test(int col, int row, int marker)
{
    if (out_of_bound(col, row)) {
        return;
    }

    grid_cell_t *cell = &ctx.grid[col][row];

    cell->marker = marker;
    cell->mode = MARKER_APPROACH;
    cell->schedule = 0;
    cell->start = time_us_64();
    cell->active = true;
}

void grid_start(int col, int row, bool override)
{
    if (out_of_bound(col, row)) {
        return;
    }

    if (!override) {
        if (ctx.grid[col][row].active || (ctx.grid[col][row].schedule > 0)) {
            return;
        }
    }

    grid_cell_t *cell = &ctx.grid[col][row];

    cell->marker = matrix_cfg->game.marker;
    cell->mode = MARKER_APPROACH;

    cell->schedule = time_us_64() + matrix_cfg->game.start_delay_ms * 1000;
}

void grid_judge(int col, int row, marker_mode_t mode)
{
    if (out_of_bound(col, row)) {
        return;
    }
    grid_cell_t *cell = &ctx.grid[col][row];
    cell->mode = mode;

    cell->start = time_us_64();
    cell->active = true;
}

void grid_abort(int col, int row)
{
    if (out_of_bound(col, row)) {
        return;
    }
    ctx.grid[col][row].active = false;
}

bool grid_is_active(int col, int row)
{
    if (out_of_bound(col, row)) {
        return false;
    }
    return ctx.grid[col][row].active;
}

int grid_last_marker(int col, int row)
{
    if (out_of_bound(col, row)) {
        return -1;
    }
    return ctx.grid[col][row].marker;
}

int grid_last_mode(int col, int row)
{
    if (out_of_bound(col, row)) {
        return -1;
    }
    return ctx.grid[col][row].mode;
}

static uint64_t update_time;

void grid_update()
{
    update_time = time_us_64();

    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            grid_cell_t *cell = &ctx.grid[col][row];
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
        }
    }
}

void grid_render()
{
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            grid_cell_t *cell = &ctx.grid[col][row];
            uint64_t elapsed = update_time - cell->start;
            if (cell->active) {
                marker_draw(col * 17, row * 17, cell->marker, cell->mode, elapsed);
            }
        }
    }
}

