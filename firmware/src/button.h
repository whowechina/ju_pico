/*
 * Ju Controller Buttons
 * WHowe <github.com/whowechina>
 */

#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/flash.h"

void button_init();

/* if anykey is pressed, no debounce */
bool button_is_stuck();

uint8_t button_num();
void button_update();
uint32_t button_read();
uint8_t button_real_gpio(int id);
uint8_t button_default_gpio(int id);

uint32_t button_stat_keydown(uint8_t id);
void button_clear_stat();

#endif
