/*
 * Controller Main
 * WHowe <github.com/whowechina>
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "bsp/board.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"

#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"

#include "tusb.h"
#include "usb_descriptors.h"

#include "board_defs.h"

#include "button.h"
#include "rgb.h"

#include "save.h"
#include "config.h"
#include "cli.h"
#include "commands.h"
#include "hid.h"

#define BUTTON_ROTATE_BIT 0x10000
static bool in_rotate_setting = false;

static void runtime_ctrl()
{
    static uint64_t rotate_setting_timeout = 0;

    static uint32_t last_button = 0;
    uint32_t new_button = button_read();
    uint32_t triggered = new_button & ~last_button;
    last_button = new_button;

    uint64_t now = time_us_64();
    if (triggered & BUTTON_ROTATE_BIT) {
        if (in_rotate_setting) {
            ju_cfg->hid.rotate = (ju_cfg->hid.rotate + 1) % 4;
        } else {
            in_rotate_setting = true;
        }
        rotate_setting_timeout = now + 5000000;
    }

    if (in_rotate_setting && (now > rotate_setting_timeout)) {
        in_rotate_setting = false;
        config_changed();
    }
}

static void run_lights()
{
    if (in_rotate_setting) {
        for (int i = 0; i < 4; i++) {
            uint32_t color = 0;
            if (ju_cfg->hid.rotate == i) {
                color = rgb32_from_hsv(i * 64, 0xff, 0xff);
            }
            rgb_set_color(i, color);
        }
        return;
    }

    uint32_t phase = time_us_32() >> 15;
    for (int i = 0; i < 8; i++) {
        uint32_t x = (phase + i * 40) & 0xff;
        rgb_set_color(i, rgb32_from_hsv(x, 0xff, 0xff));
    }
}

static mutex_t core1_io_lock;
static void core1_loop()
{
    while (1) {
        if (mutex_try_enter(&core1_io_lock, NULL)) {
            run_lights();
            rgb_update();
            mutex_exit(&core1_io_lock);
        }
        cli_fps_count(1);
        sleep_ms(1);
    }
}

static void core0_loop()
{
    uint64_t next_frame = time_us_64();

    while(1) {
        tud_task();

        cli_run();
        save_loop();
        cli_fps_count(0);

        sleep_until(next_frame);
        next_frame += 1000; // 1KHz

        button_update();
        hid_update();

        runtime_ctrl();
    }
}


/* if certain key pressed when booting, enter update mode */
static void update_check()
{
    const uint8_t pins[] = BUTTON_DEF;
    int pressed = 0;
    for (int i = 0; i < count_of(pins); i++) {
        uint8_t gpio = pins[i];
        gpio_init(gpio);
        gpio_set_function(gpio, GPIO_FUNC_SIO);
        gpio_set_dir(gpio, GPIO_IN);
        gpio_pull_up(gpio);
        sleep_ms(1);
        if (!gpio_get(gpio)) {
            pressed++;
        }
    }

    if (pressed >= 4) {
        sleep_ms(100);
        reset_usb_boot(0, 2);
        return;
    }
}

void init()
{
    sleep_ms(50);
    board_init();

    update_check();

    tusb_init();
    stdio_init_all();

    config_init();
    mutex_init(&core1_io_lock);

    save_init(board_id_32() ^ 0xcafe1111, &core1_io_lock);

    button_init();
    rgb_init();

    cli_init("ju_pico>", "\n   << Ju Pico Controller >>\n"
                            " https://github.com/whowechina\n\n");
    commands_init();

    ju_runtime.key_stuck = button_is_stuck();
}

int main(void)
{
    init();
    multicore_launch_core1(core1_loop);
    core0_loop();
    return 0;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen)
{
    printf("Get from USB %d-%d\n", report_id, report_type);
    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize)
{
    hid_proc(buffer, bufsize);
}
