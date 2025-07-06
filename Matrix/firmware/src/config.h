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
        uint8_t level;
        uint8_t reserved[15];
    } light;
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
