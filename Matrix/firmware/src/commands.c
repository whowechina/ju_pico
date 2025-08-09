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
#include "grid.h"
#include "hub75.h"
#include "resource.h"

static void disp_panel()
{
    printf("[Panel]\n");
    printf("  FM6126: %s\n", matrix_cfg->panel.fm6126 ? "Yes" : "No");
    printf("  RGB Order: %s\n", matrix_cfg->panel.rgb_order == 0 ? "RGB" : "RBG");
}

static void disp_game()
{
    printf("[Game]\n");
    printf("  Marker Number: %d\n", marker_num());
    printf("  Current Marker: %d\n", matrix_cfg->game.marker);
    printf("  Start Delay: %d ms\n", matrix_cfg->game.start_delay_ms);
    printf("  Background Color: 0x%06lx\n", matrix_cfg->game.color.background);
    printf("  Combo Color: 0x%06lx\n", matrix_cfg->game.color.combo);
    printf("  Score Color: 0x%06lx\n", matrix_cfg->game.color.score);
    printf("  Combo Font: %d\n", matrix_cfg->game.font.combo);
    printf("  Score Font: %d\n", matrix_cfg->game.font.score);
}

void handle_display(int argc, char *argv[])
{
    const char *usage = "Usage: display [panel|game]\n";
    if (argc > 1) {
        printf(usage);
        return;
    }

    const char *choices[] = {"panel", "game"};
    static void (*disp_funcs[])() = {
        disp_panel,
        disp_game,
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

    matrix_cfg->panel.level = level;
    config_changed();
    disp_panel();
}

static void handle_marker(int argc, char *argv[])
{
    const char *usage = "Usage: marker show <0|1>\n"
                        "       marker set <0..%d>\n"
                        "       marker delay <0..1000>\n"
                        " Note: delay is in milliseconds.\n";

    if (argc != 2) {
        printf(usage, marker_num() - 1);
        return;
    }

    const char *choices[] = {"show", "set", "delay"};
    int choice = cli_match_prefix(choices, count_of(choices), argv[0]);
    if (choice < 0) {
        printf(usage, marker_num() - 1);
        return;
    }

    if (choice == 0) {
        int group = cli_extract_non_neg_int(argv[1], 0);
        if (group < 0 || group > 1) {
            printf(usage, marker_num() - 1);
            return;
        }

        for (int i = 0; (i < 16) && (i < marker_num()); i++) {
            grid_test(i % 4, i / 4, i + group * 16);
        }
        return;
    }
    
    if (choice == 1) {
        int marker = cli_extract_non_neg_int(argv[1], 0);
        if ((marker < 0) || (marker >= marker_num())) {
            printf(usage, marker_num() - 1);
            return;
        }

        matrix_cfg->game.marker = marker;
        grid_schedule(marker % 4, marker / 4 % 4, true);
    } else if (choice == 2) {
        int delay = cli_extract_non_neg_int(argv[1], 0);
        if ((delay < 0) || (delay > 1000)) {
            printf(usage, marker_num() - 1);
            return;
        }

        matrix_cfg->game.start_delay_ms = delay;
    }
    config_changed();
    disp_game();
}

static bool parse_color(uint32_t *color, int argc, char *argv[])
{
    if (argc != 4) {
        return false;
    }

    const char *formats[] = {"rgb", "hsv"};
    int format = cli_match_prefix(formats, count_of(formats), argv[0]);
    if (format < 0) {
        return false;
    }

    int a = cli_extract_non_neg_int(argv[1], 0);
    int b = cli_extract_non_neg_int(argv[2], 0);
    int c = cli_extract_non_neg_int(argv[3], 0);

    if (a < 0 || a > 255 || b < 0 || b > 255 || c < 0 || c > 255) {
        return false;
    }

    if (format == 0) {
        *color = (a << 16) | (b << 8) | c;
    } else {
        *color = hub75_hsv2rgb(a, b, c);
    }
    return true;
}

static void handle_color(int argc, char *argv[])
{
    const char *usage = "Usage: color <background|combo|score> rgb r g b\n"
                        "       color <background|combo|score> hsv h s v\n";
    
    if (argc != 5) {
        printf(usage);
        return;
    }

    const char *choices[] = {"background", "combo", "score"};
    int choice = cli_match_prefix(choices, count_of(choices), argv[0]);
    if (choice < 0) {
        printf(usage);
        return;
    }

    uint32_t color = 0;
    if (!parse_color(&color, argc - 1, argv + 1)) {
        printf(usage);
        return;
    }

    if (choice == 0) {
        matrix_cfg->game.color.background = color;
    } else if (choice == 1) {
        matrix_cfg->game.color.combo = color;
    } else if (choice == 2) {
        matrix_cfg->game.color.score = color;
    }

    config_changed();
    disp_game();
}

static void handle_font(int argc, char *argv[])
{
    const char *usage = "Usage: font <combo|score> <0..%d>\n";
    if (argc != 2) {
        printf(usage, font_count - 1);
        return;
    }

    const char *choices[] = {"combo", "score"};
    int choice = cli_match_prefix(choices, count_of(choices), argv[0]);
    if (choice < 0) {
        printf(usage, font_count - 1);
        return;
    }

    int font_index = cli_extract_non_neg_int(argv[1], 0);
    if ((font_index < 0) || (font_index >= font_count)) {
        printf(usage, font_count - 1);
        return;
    }

    if (choice == 0) {
        matrix_cfg->game.font.combo = font_index;
    } else if (choice == 1) {
        matrix_cfg->game.font.score = font_index;
    }
    
    config_changed();
    disp_game();
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
    cli_register("marker", handle_marker, "Set marker.");
    cli_register("color", handle_color, "Set colors.");
    cli_register("font", handle_font, "Set fonts.");
    cli_register("save", handle_save, "Save config to flash.");
    cli_register("factory", config_factory_reset, "Reset everything to default.");
}
