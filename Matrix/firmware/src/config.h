/*
 * Controller Config
 * WHowe <github.com/whowechina>
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#include "board_defs.h"

typedef struct __attribute__((packed)) {
    struct {
        bool fm6126;
        uint8_t rgb_order; // 0: RGB, 1: RBG
        uint8_t level;
        uint8_t reserved[13];
    } panel;
    struct {
        uint8_t marker;
        uint16_t start_delay_ms;
        struct {
            uint32_t background;
            uint32_t combo;
            uint32_t score;
            uint32_t reserved[5];
        } color;
        struct {
            uint8_t combo;
            uint8_t score;
            uint8_t reserved[6];
        } font;
    } game;
    struct {
    } hid;
    uint8_t reserved[8];
} matrix_cfg_t;

typedef struct {
    uint16_t fps[2];
    bool key_stuck;
} matrix_runtime_t;

extern matrix_cfg_t *matrix_cfg;
extern matrix_runtime_t matrix_runtime;

void config_init();
void config_changed(); // Notify the config has changed
void config_factory_reset(); // Reset the config to factory default

#endif
