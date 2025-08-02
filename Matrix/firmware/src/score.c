#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "hub75.h"
#include "score.h"
#include "resource.h"

static struct {
    uint32_t score;
    uint32_t combo;
} score_ctx;

void score_draw_combo()
{
    const font_t *font = &font_lib[0];
    char str[24];
    snprintf(str, sizeof(str), "%lu", score_ctx.combo);

    font_draw_string(font, 32, 17, str, matrix_cfg->game.color.combo, ALIGN_CENTER, -1);
}

void score_draw_score()
{
    const font_t *font = &font_lib[0];
    char str[24];
    snprintf(str, sizeof(str), "%lu", score_ctx.score);

    font_draw_string(font, 32, 17, str, matrix_cfg->game.color.score, ALIGN_CENTER, -1);
}

void score_set_score(uint32_t score)
{
    score_ctx.score = score;
}

void score_set_combo(uint32_t combo)
{
    score_ctx.combo = combo;
}
