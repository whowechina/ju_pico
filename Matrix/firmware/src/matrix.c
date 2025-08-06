#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/time.h"

#include "cli.h"
#include "matrix.h"
#include "hub75.h"
#include "marker.h"
#include "score.h"
#include "grid.h"
#include "resource.h"
#include "font.h"
#include "config.h"
#include "ubthax.h"

static void dma_complete(uint16_t row, uint16_t bit)
{
    if (row == 0 && bit == 0) {
        cli_fps_count(2);
    }
}

void matrix_init()
{
    hub75_init(matrix_cfg->panel.fm6126, false);
    hub75_start(dma_complete);
}

static void run_idle()
{
    if (rand() % 10000 < 10) {
        int col = rand() % 4;
        int row = rand() % 4;
        if (!grid_is_active(col, row)) {
            grid_test(col, row, rand() % marker_num());
        }
    }

    grid_update();
    grid_render();
}

static void run_preview()
{
    grid_render_preview();
}

static void run_game()
{
    grid_update();
    score_draw_combo();
    grid_render();
}

static void run_result()
{
    score_draw_final();
}

void matrix_update()
{
    if (hub75_is_paused()) {
        hub75_resume();
    }

    hub75_fill(matrix_cfg->game.color.background);
    ubt_phase_t phase = ubthax_get_phase();
    if (phase == UBT_STARTING) {
        run_preview();
    } else if (phase == UBT_INGAME) {
        run_game();
    } else if (phase == UBT_RESULT) {
        run_result();
    } else {
        run_idle();
    }
    hub75_update();
}

void matrix_pause()
{
    if (!hub75_is_paused()) {
        hub75_pause();
    }
}
