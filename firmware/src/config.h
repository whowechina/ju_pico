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
        int8_t filter;
        int8_t global;
        uint8_t debounce_touch;
        uint8_t debounce_release;        
        int8_t zones[16];
    } sense;
    uint8_t reserved[8];
} ju_cfg_t;

typedef struct {
    uint16_t fps[2];
    bool key_stuck;
} ju_runtime_t;

extern ju_cfg_t *ju_cfg;
extern ju_runtime_t ju_runtime;

void config_init();
void config_changed(); // Notify the config has changed
void config_factory_reset(); // Reset the config to factory default

#endif
