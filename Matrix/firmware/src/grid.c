#include <stdlib.h>
#include <stdint.h>
#include "pico/time.h"

#include "marker.h"
#include "grid.h"

typedef struct {
    bool active;
    int8_t marker;
    int8_t mode;
    uint64_t start;
} grid_cell_t;

static struct {
    uint8_t marker;
    grid_cell_t grid[4][4];
} ctx;

void grid_set_marker(int marker)
{
    if ((marker < 0) || (marker >= marker_num())) {
        return;
    }

    ctx.marker = marker % marker_num();
}

static inline bool out_of_bound(int col, int row)
{
    return (col < 0) || (col >= 4) || (row < 0) || (row >= 4);
}

void grid_start(int col, int row)
{
    if (out_of_bound(col, row)) {
        return;
    }

    grid_cell_t *cell = &ctx.grid[col][row];

    cell->marker = ctx.marker;
    cell->mode = MARKER_APPROACH;

    cell->start = time_us_64();
    cell->active = true;
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

void grid_render()
{
    uint64_t now = time_us_64();
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            grid_cell_t *cell = &ctx.grid[col][row];
            uint64_t elapsed = now - cell->start;
            if (marker_is_end(cell->marker, cell->mode, elapsed)) {
                cell->active = false;
            }
            if (cell->active) {
                marker_draw(col * 17, row * 17, cell->marker, cell->mode, elapsed);
            }
        }
    }
}

