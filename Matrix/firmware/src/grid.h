/*
    4x4 Grid System
*/
#ifndef GRID_H
#define GRID_H

#include <stdint.h>
#include "marker.h"

void grid_set_marker(int marker);

void grid_init();

void grid_test(int col, int row, int marker);

void grid_preview(int col, int row);
void grid_preview_reset();

void grid_start(int col, int row, bool override);
void grid_judge(int col, int row, marker_mode_t mode);
void grid_abort(int col, int row);

bool grid_is_active(int col, int row);
int grid_last_marker(int col, int row);
int grid_last_mode(int col, int row);

void grid_update();
void grid_render();
void grid_render_preview();

typedef void (*grid_marker_finish_cb)(int col, int row, marker_mode_t mode);
void grid_listen(grid_marker_finish_cb on_finish);



#endif
