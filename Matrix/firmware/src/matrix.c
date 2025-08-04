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

void draw_button(uint8_t x, uint8_t y, uint32_t border, uint32_t solid)
{
    int col = x * 17;
    int row = y * 17;
    for (int i = 0; i < 13; i++) {
        hub75_pixel(col + i, row, border);
        hub75_pixel(col + i, row + 12, border);
        hub75_pixel(col, row + i, border);
        hub75_pixel(col + 12, row + i, border);
    }

    for (int i = 1; i < 12; i++) {
        for (int j = 1; j < 12; j++) {
            hub75_pixel(col + i, row + j, solid);
        }
    }
}

void draw_door(uint8_t x, uint8_t y, uint32_t progress, bool arrow)
{
    const uint32_t solid = hub75_rgb(0, 140, 140);
    int col = x * 17;
    int row = y * 17;
    for (int i = 0; i < 13; i++) {
        int len = arrow ? abs(i - 6) + 1 : 0;
        int bottom = len + 11;

        int prog = progress * 255 * 12 / 1000;
 
        int split = prog / 255;
        int extra = prog % 255;
        for (int j = 0; j < 13; j++) {
            uint32_t color = 0;
            if (j < len) {
                color = solid;
            } else if (j > bottom - split) {
                color = solid;
            }
            hub75_pixel(col + i, row + j, color);
        }
        if (bottom - split < 13) {
            hub75_blend(col + i, row + bottom - split, hub75_alpha(extra, solid));
        }
    }
}

void draw_sector_fast(uint8_t x, uint8_t y, uint32_t progress)
{
    const uint32_t solid = hub75_rgb(255, 100, 50);
    int col = x * 17;
    int row = y * 17;
    
    int center = 6;
    
    // 进度转换为角度 (0-360度，用整数避免浮点运算)
    int angle_max = (progress * 360) / 1000;
    
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            int dx = i - center;
            int dy = j - center;
            
            // 计算角度(用整数近似，从12点方向开始顺时针)
            int angle;
            if (dx == 0 && dy < 0) angle = 0;     // 正北
            else if (dx > 0 && dy < 0) angle = 45;   // 东北
            else if (dx > 0 && dy == 0) angle = 90;  // 正东
            else if (dx > 0 && dy > 0) angle = 135;  // 东南
            else if (dx == 0 && dy > 0) angle = 180; // 正南
            else if (dx < 0 && dy > 0) angle = 225;  // 西南
            else if (dx < 0 && dy == 0) angle = 270; // 正西
            else if (dx < 0 && dy < 0) angle = 315;  // 西北
            else angle = 0; // 中心点
            
            // 更精细的角度计算（可选）
            if (dx != 0 || dy != 0) {
                // 简化的角度计算，用查表法或线性插值
                if (abs(dx) > abs(dy)) {
                    // 主要在东西方向
                    if (dx > 0) {
                        angle = 90 + (dy * 45) / abs(dx);
                    } else {
                        angle = 270 - (dy * 45) / abs(dx);
                    }
                } else {
                    // 主要在南北方向
                    if (dy < 0) {
                        angle = 0 + (dx * 45) / abs(dy);
                    } else {
                        angle = 180 - (dx * 45) / abs(dy);
                    }
                }
                if (angle < 0) angle += 360;
            }
            
            bool in_sector = (angle <= angle_max) || (angle_max >= 360);
            
            // 边缘抗锯齿
            if (!in_sector && angle_max < 360) {
                int angle_diff = angle - angle_max;
                if (angle_diff < 10 && angle_diff > 0) {  // 10度的渐变区域
                    uint8_t alpha = 255 * (10 - angle_diff) / 10;
                    hub75_blend(col + i, row + j, hub75_alpha(alpha, solid));
                }
            } else if (in_sector) {
                hub75_pixel(col + i, row + j, solid);
            }
        }
    }
}

void draw_closing_door_smooth(uint8_t x, uint8_t y, uint32_t progress, bool invert)
{
    const uint32_t solid = hub75_rgb(100, 255, 100);
    int col = x * 17;
    int row = y * 17;
    
    // 精确的关门进度 (0-6.5的浮点数效果，用整数模拟)
    int close_distance_x10 = (progress * 65) / 1000;  // 0-65 (代表0-6.5)
    int full_close = close_distance_x10 / 10;
    int partial_close = close_distance_x10 % 10;
    
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            // 计算到中心或边缘的距离
            int distance = invert ? 
                ((j <= 6) ? j : (12 - j)) :  // 距离边缘
                abs(j - 6);                  // 距离中心
            
            // 直接计算并绘制
            if (distance < full_close) {
                hub75_pixel(col + i, row + j, solid);
            } else if (distance == full_close && full_close <= 6) {
                uint8_t alpha = (partial_close * 255) / 10;
                hub75_blend(col + i, row + j, hub75_alpha(alpha, solid));
            }
        }
    }
}

void rotate_90(int x, int y, int times)
{
    times = times % 4;
    
    if (times == 0) {
        return;
    }

    int col = x * 17;
    int row = y * 17;

    uint32_t temp[13][13];
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            temp[i][j] = canvas[col + i][row + j];
        }
    }
    
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            int src_i;
            int src_j;

            if (times == 1) {
                src_i = 12 - j;
                src_j = i;
            } else if (times == 2) {
                src_i = 12 - i;
                src_j = 12 - j;
            } else {
                src_i = j;
                src_j = 12 - i;
            }

            canvas[col + i][row + j] = temp[src_i][src_j];
        }
    }
}

static void run_idle()
{
    for (int x = 0; x < PANEL_WIDTH; x++) {
        for (int y = 0; y < PANEL_HEIGHT; y++) {
            hub75_pixel(x, y, hub75_rgb(x, y, x+y));
        }
    }
    grid_update();
    grid_render();
}

static void run_preview()
{
    score_set_combo(0);
    score_set_score(0);
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
    score_draw_score();
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
