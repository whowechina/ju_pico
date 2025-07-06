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

matrix_cfg_t *matrix_cfg;

static matrix_cfg_t default_cfg = {
    .light = {
        .level = 0x80,
    },
};

matrix_runtime_t matrix_runtime;

static inline bool in_range(int val, int min, int max)
{
    return (val >= min) && (val <= max);
}

static void config_loaded()
{
}

void config_changed()
{
    save_request(false);
}

void config_factory_reset()
{
    *matrix_cfg = default_cfg;
    save_request(true);
}

void config_init()
{
    matrix_cfg = (matrix_cfg_t *)save_alloc(sizeof(*matrix_cfg), &default_cfg, config_loaded);
}
