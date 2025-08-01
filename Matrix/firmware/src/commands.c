#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"

#include "tusb.h"

#include "button.h"
#include "config.h"
#include "save.h"
#include "cli.h"
#include "matrix.h"

static void disp_panel()
{
    printf("[Panel]\n");
    printf("  FM6126: %s\n", matrix_cfg->panel.fm6126 ? "Yes" : "No");
    printf("  RGB Order: %s\n", matrix_cfg->panel.rgb_order == 0 ? "RGB" : "RBG");
}

static void disp_light()
{
    printf("[Light]\n");
    printf("  Level: %d\n", matrix_cfg->light.level);
}

void handle_display(int argc, char *argv[])
{
    const char *usage = "Usage: display [panel|light]\n";
    if (argc > 1) {
        printf(usage);
        return;
    }

    const char *choices[] = {"panel", "light"};
    static void (*disp_funcs[])() = {
        disp_panel,
        disp_light,
    };
  
    static_assert(count_of(choices) == count_of(disp_funcs),
        "Choices and disp_funcs arrays must have the same number of elements");

    if (argc == 0) {
        for (int i = 0; i < count_of(disp_funcs); i++) {
            disp_funcs[i]();
        }
        return;
    }

    int choice = cli_match_prefix(choices, count_of(choices), argv[0]);
    if (choice < 0) {
        printf(usage);
        return;
    }

    if (choice > count_of(disp_funcs)) {
        return;
    }

    disp_funcs[choice]();
}

static void handle_panel(int argc, char *argv[])
{
    const char *usage = "Usage: panel model <regular|fm6126>\n"
                         "      panel order <rgb|rbg>\n";
    if (argc != 2) {
        printf(usage);
        return;
    }

    const char *choices[] = {"model", "order"};
    int choice = cli_match_prefix(choices, count_of(choices), argv[0]);

    if (choice < 0) {
        printf(usage);
        return;
    }

    if (choice == 0) {
        const char *models[] = {"regular", "fm6126"};
        int model = cli_match_prefix(models, count_of(models), argv[1]);
        if (model < 0) {
            printf(usage);
            return;
        }
        matrix_cfg->panel.fm6126 = (model == 1);
    } else if (choice == 1) {
        const char *orders[] = {"rgb", "rbg"};
        int order = cli_match_prefix(orders, count_of(orders), argv[1]);
        if (order < 0) {
            printf(usage);
            return;
        }
        matrix_cfg->panel.rgb_order = order;
    } else {
        printf(usage);
        return;
    }

    config_changed();
    disp_panel();
    printf("You may need to reboot the device to apply the panel changes.\n");
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

    matrix_cfg->light.level = level;
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
    cli_register("panel", handle_panel, "Set panel config.");
    cli_register("level", handle_level, "Set LED brightness level.");
    cli_register("save", handle_save, "Save config to flash.");
    cli_register("factory", config_factory_reset, "Reset everything to default.");
}
