#include <stdlib.h>
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
    if ((row == 0) && (bit == 0)) {
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
    
    if (rand() % 10000 < 20) {
        int col = rand() % 4;
        int row = rand() % 4;
        if (!grid_is_active(col, row)) {
    //        grid_test(col, row, rand() % marker_num());
        }
    }

    score_set_combo(123);
    score_draw_combo();

    uint32_t now = time_us_32() / 1000;
    if (now / 100 % 50 == 0) {
        if (!grid_is_active(3, 1)) {
            grid_schedule(3, 1, true);
            grid_attach_trail(3, 1, 2, 3, 1666, true);
        }
    } else if (now / 100 % 50 == 8) {
        if (grid_is_started(3, 1)) {
            if (!grid_is_moving(3, 1)) {
                grid_judge(3, 1, MARKER_PERFECT);
            }
        }
    }

    grid_update();
    grid_render();
}

static void run_preview()
{
    grid_render_preview(0xff);
}

static void run_game()
{
    int elapsed = ubthax_get_phase_time_ms();
    int fade_time = matrix_cfg->game.start_delay_ms;
    if ((fade_time > 0) && (elapsed < fade_time)) {
        int alpha = (fade_time - elapsed) * 255 / fade_time;
        grid_render_preview(alpha);
    }
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
