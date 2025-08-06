#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "pico/time.h"

#include "config.h"
#include "hub75.h"
#include "font.h"
#include "score.h"

static struct {
    uint32_t score;
    uint32_t combo;
    uint64_t start_time;
    const char *title;
} score_ctx;

const uint32_t title_ani_duration = 500000;
const uint32_t ranking_start = 1000000;
const uint32_t ranking_fade_in = 500000;

void score_draw_combo()
{
    if (score_ctx.combo < 5) {
        return;
    }

    char str[24];
    snprintf(str, sizeof(str), "%lu", score_ctx.combo);
    uint32_t color = matrix_cfg->game.color.combo | 0xff000000;
    uint32_t x_pos = PANEL_WIDTH / 2;
    uint32_t y_pos = PANEL_HEIGHT / 4 + 1;
    font_spacing(-1, -1);
    font_draw_text(x_pos, y_pos, str, 0, color, ALIGN_CENTER);
}

int calc_brake_pos(int start, int end, uint32_t duration, uint32_t elapsed)
{
    if (elapsed >= duration) {
        return end;
    }
    
    int distance = end - start;
    uint32_t t = (elapsed * 1000) / duration;

    uint32_t left = 1000 - t;

    uint32_t cubic = left;
    cubic = (cubic * left) / 1000;
    cubic = (cubic * left) / 1000;

    int progress = 1000 - cubic;
    
    return start + (distance * progress) / 1000;
}

static void score_draw_score(uint8_t alpha)
{
    char str[24];
    snprintf(str, sizeof(str), "%lu", score_ctx.score);
    uint32_t x_pos = PANEL_WIDTH / 2;
    uint32_t y_pos = PANEL_HEIGHT * 3 / 4 + 1;
    uint32_t color = matrix_cfg->game.color.score | (alpha << 24);
    font_spacing(0, 0);
    font_draw_text(x_pos, y_pos, str, 1, color, ALIGN_CENTER);
}

static int get_rank(uint32_t score)
{
    if (score >= 1000000) {
        return 0;
    } else if (score >= 980000) {
        return 1;
    } else if (score >= 950000) {
        return 2;
    } else if (score >= 900000) {
        return 3;
    } else if (score >= 850000) {
        return 4;
    } else if (score >= 800000) {
        return 5;
    } else if (score >= 700000) {
        return 6;
    } else if (score >= 500000) {
        return 7;
    } else {
        return 8;
    }
}

static void score_draw_rank(uint8_t alpha)
{
    int rank = get_rank(score_ctx.score);
    const char *judge = rank <= 6 ? RGB("\x00\xc0\xc0" "CLEARED") :
                                    RGB("\xd0\x20\x20" "FAILED");
    font_spacing(0, 0);
    font_draw_text(PANEL_WIDTH / 2, PANEL_HEIGHT / 4,
                   judge, 2, alpha << 24, ALIGN_CENTER);

    const char *ranks[] = {
        SPACING("\xfd\x00") RGB("\xff\x80\x80") "E" RGB("\xa0\xa0\xff") "X" RGB("\x80\xff\x80") "C",
        SPACING("\xfd\x00") RGB("\xd0\xd0\x00") "S" RGB("\x00\xd0\xd0") "S" RGB("\xd0\x00\xd0") "S",
        SPACING("\xff\x00") RGB("\x00\xc0\xc0") "SS",
        RGB("\xc0\xc0\x00") "S",
        RGB("\x00\xc0\xc0") "A",
        RGB("\xc0\xa0\x00") "B",
        RGB("\x00\xc0\xc0") "C",
        RGB("\xa0\x80\x80") "D",
        RGB("\xa0\x40\x40") "E",
    };

    const char *rank_str = ranks[rank];
    
    font_spacing(0, 0);
    font_draw_text(PANEL_WIDTH * 5 / 8, PANEL_HEIGHT / 2 + 1,
                   rank_str, 2, alpha << 24, ALIGN_CENTER);
}

static inline uint8_t ease_in(uint8_t alpha)
{
    return (alpha * alpha) / 255;
}

static void score_draw_title()
{
    uint64_t elapsed = time_us_64() - score_ctx.start_time;

    int center = PANEL_HEIGHT * 3 / 8;
    int pos_up = calc_brake_pos(center, -1, title_ani_duration, elapsed);

    int alpha = 255;
    if (elapsed < title_ani_duration) {
        alpha = ease_in(elapsed * 255 / title_ani_duration);
    }
    int pos_dn = calc_brake_pos(center, PANEL_HEIGHT * 7 / 8 - 6, title_ani_duration, elapsed);

    font_draw_text(PANEL_WIDTH / 2, pos_up, score_ctx.title, 2, alpha << 24, ALIGN_CENTER);

    if (elapsed >= ranking_start) {
        alpha = 0;
        elapsed -= title_ani_duration;
        if (elapsed < ranking_fade_in) {
            alpha = ease_in((ranking_fade_in - elapsed) * 255 / ranking_fade_in);
        }
    }

    font_draw_text(PANEL_WIDTH / 2, pos_dn, score_ctx.title, 2, alpha << 24, ALIGN_CENTER);
}

void score_draw_final()
{
    score_draw_title();
    
    uint64_t elapsed = time_us_64() - score_ctx.start_time;
    if (elapsed > ranking_start) {
        int alpha = 255;
        elapsed -= ranking_start;
        if (elapsed < ranking_fade_in) {
            alpha = ease_in(elapsed * 255 / ranking_fade_in);
        }
        score_draw_score(alpha);
        score_draw_rank(alpha);
    }
}

void score_set_score(uint32_t score)
{
    score_ctx.score = score;
}

void score_set_final_score(uint32_t score)
{
    score_ctx.score = score;
    score_ctx.start_time = time_us_64();
    score_ctx.title = RGB("\xc0\xc0\x10") "RESULT";
}

void score_set_fullcombo()
{
    score_ctx.title = SPACING("\x00\x00")
                      RGB("\xe1\xad\xad") "F"
                      RGB("\xe1\xc7\xad") "-"
                      RGB("\xc7\xe1\xad") "C"
                      RGB("\xad\xe1\xad") "O"
                      RGB("\xad\xe1\xc7") "M"
                      RGB("\xad\xc7\xe1") "B"
                      RGB("\xad\xad\xe1") "O";
}

void score_set_excellent()
{
    score_ctx.title = SPACING("\xff\x00")
                      RGB("\xdc\x5a\x5a") "E"
                      RGB("\xdc\xb0\x5a") "X"
                      RGB("\xb0\xdc\x5a") "C"
                      RGB("\x5a\xdc\x5a") "E"
                      RGB("\x5a\xdc\xb0") "L"
                      RGB("\x5a\xb0\xdc") "L"
                      RGB("\x5a\x5a\xdc") "E"
                      RGB("\xb0\x5a\xdc") "N"
                      RGB("\xdc\x5a\xb0") "T";
}

void score_set_combo(uint32_t combo)
{
    score_ctx.combo = combo;
}

