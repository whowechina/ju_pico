/*
 * Controller Config and Runtime Data
 * WHowe <github.com/whowechina>
 * 
 * Config is a global data structure that stores all the configuration
 * Runtime is something to share between files.
 */

#include <string.h>

#include "config.h"
#include "save.h"
#include "touch.h"

ju_cfg_t *ju_cfg;

static ju_cfg_t default_cfg = {
    .light = {
        .level = 0x80,
    },
    .sense = {
        .filter = 0x10,
        .debounce_touch = 1,
        .debounce_release = 2,
    },
    .hid = {
        .rotate = 0,
        .report_rotate = true,
    },
};

ju_runtime_t ju_runtime;

static inline bool in_range(int val, int min, int max)
{
    return (val >= min) && (val <= max);
}

static void config_loaded()
{
    if ((ju_cfg->sense.filter & 0x0f) > 3 ||
        ((ju_cfg->sense.filter >> 4) & 0x0f) > 3) {
        ju_cfg->sense = default_cfg.sense;
        config_changed();
    }
    if (!in_range(ju_cfg->sense.global, -9, 9)) {
        ju_cfg->sense = default_cfg.sense;
        config_changed();
    }
    for (int i = 0; i < 16; i++) {
        if (!in_range(ju_cfg->sense.zones[i], -9, 9)) {
            ju_cfg->sense = default_cfg.sense;
            config_changed();
            break;
        }
    }
    if (!in_range(ju_cfg->sense.debounce_touch, 0, 7) ||
        !in_range(ju_cfg->sense.debounce_release, 0, 7)) {
        ju_cfg->sense = default_cfg.sense;
        config_changed();
    }
}

void config_changed()
{
    save_request(false);
}

void config_factory_reset()
{
    *ju_cfg = default_cfg;
    save_request(true);
}

void config_init()
{
    ju_cfg = (ju_cfg_t *)save_alloc(sizeof(*ju_cfg), &default_cfg, config_loaded);
}
