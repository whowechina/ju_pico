#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "tusb.h"

#include "touch.h"
#include "button.h"
#include "config.h"
#include "save.h"
#include "cli.h"

#define SENSE_LIMIT_MAX 9
#define SENSE_LIMIT_MIN -9

static void disp_light()
{
    printf("[Light]\n");
    printf("  Level: %d\n", ju_cfg->light.level);
}

static void disp_sense()
{
    printf("[Sense]\n");
    printf("  Filter: %u, %u, %u\n", ju_cfg->sense.filter >> 6,
                                    (ju_cfg->sense.filter >> 4) & 0x03,
                                    ju_cfg->sense.filter & 0x07);
    printf("  Sensitivity (global: %+d):\n", ju_cfg->sense.global);
    printf("  ");
    for (int i = 0; i < 16; i++) {
        printf(" %2d,", ju_cfg->sense.zones[i]);
    }
    printf("\n");
    printf("  Debounce (touch, release): %d, %d\n",
           ju_cfg->sense.debounce_touch, ju_cfg->sense.debounce_release);
}

static void disp_touch()
{
    printf("[Touch]\n");
}

#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))

void handle_display(int argc, char *argv[])
{
    const char *usage = "Usage: display [light|sense|touch]\n";
    if (argc > 1) {
        printf(usage);
        return;
    }

    const char *choices[] = {"light", "sense", "touch"};
    static void (*disp_funcs[])() = {
        disp_light,
        disp_sense,
        disp_touch,
    };
  
    static_assert(ARRAYSIZE(choices) == ARRAYSIZE(disp_funcs),
        "Choices and disp_funcs arrays must have the same number of elements");

    if (argc == 0) {
        for (int i = 0; i < ARRAYSIZE(disp_funcs); i++) {
            disp_funcs[i]();
        }
        return;
    }

    int choice = cli_match_prefix(choices, ARRAYSIZE(choices), argv[0]);
    if (choice < 0) {
        printf(usage);
        return;
    }

    if (choice > ARRAYSIZE(disp_funcs)) {
        return;
    }

    disp_funcs[choice]();
}

static void handle_level(int argc, char *argv[])
{
    const char *usage = "Usage: level <0..255>\n";
    if (argc != 1) {
        printf(usage);
        return;
    }

    int level = cli_extract_non_neg_int(argv[0], 0);
    if ((level < 0) || (level > 255)) {
        printf(usage);
        return;
    }

    ju_cfg->light.level = level;
    config_changed();
    disp_light();
}

static void handle_save()
{
    save_request(true);
}

void commands_init()
{
    cli_register("display", handle_display, "Display all config.");
    cli_register("level", handle_level, "Set LED brightness level.");
    cli_register("save", handle_save, "Save config to flash.");
    cli_register("factory", config_factory_reset, "Reset everything to default.");
}
