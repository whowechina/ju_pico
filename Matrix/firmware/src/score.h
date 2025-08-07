#ifndef SCORE_H
#define SCORE_H

#include <stdint.h>
#include <stdint.h>

void score_draw_combo();
void score_draw_final();

void score_set_score(uint32_t score);
void score_set_combo(uint32_t combo);

void score_end_result();
void score_end_fullcombo();
void score_end_excellent();

#endif
