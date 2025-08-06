#ifndef SCORE_H
#define SCORE_H

#include <stdint.h>
#include <stdint.h>

void score_draw_combo();
void score_draw_final();

void score_set_score(uint32_t score);
void score_set_final_score(uint32_t score);
void score_set_fullcombo();
void score_set_excellent();
void score_set_combo(uint32_t combo);

#endif
