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
#include "hardware/clocks.h"

#include "tusb.h"
#include "usb_descriptors.h"

#include "board_defs.h"

#include "button.h"

#include "save.h"
#include "config.h"
#include "cli.h"
#include "commands.h"
#include "ubthax.h"
#include "matrix.h"

static mutex_t core1_io_lock;
static void core1_loop()
{
    while (1) {
        if (mutex_try_enter(&core1_io_lock, NULL)) {
            matrix_update();
            mutex_exit(&core1_io_lock);
        } else {
            matrix_pause();
            sleep_ms(2);
        }
        sleep_us(100);
        cli_fps_count(1);
    }
}

static void runtime_ctrl()
{
    if (button_read()) {
        sleep_ms(100);
        reset_usb_boot(0, 2);
        sleep_ms(1000);
    }
}

struct __attribute__((packed)) {
    uint8_t buttons;
} hid_report = {0};

static void report_usb_hid()
{
    if (tud_hid_ready()) {
        hid_report.buttons = button_read() & 0xff;
        tud_hid_n_report(0, REPORT_ID_JOYSTICK, &hid_report, sizeof(hid_report));
    }
}

void hid_update()
{
    report_usb_hid();
}

static void core0_loop()
{
    uint64_t next_frame = time_us_64();
    while(1) {
        tud_task();

        cli_run();
        save_loop();
        cli_fps_count(0);

        button_update();
        hid_update();

        runtime_ctrl();
        ubthax_update();

        sleep_until(next_frame);
        next_frame += 1001;
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

    if (pressed >= 1) {
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

    set_sys_clock_khz(200000, false);

    tusb_init();
    stdio_init_all();

    config_init();
    mutex_init(&core1_io_lock);

    save_init(board_id_32() ^ 0xcafe1112, &core1_io_lock);

    button_init();

    cli_fps_label(2, "LED");

    ubthax_init();
    matrix_init();

    cli_init("ju_matrix>",
#ifdef BOARD_JU_MATRIX
             "\n   << Ju Matrix >>\n"
#else
             "\n   << Ju Matrix Plus >>\n"
#endif
            " https://github.com/whowechina\n\n");

    commands_init();

    matrix_runtime.key_stuck = button_is_stuck();
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
    if ((report_type == HID_REPORT_TYPE_FEATURE) &&
        (report_id == REPORT_ID_JUBEATHAX)) {
        ubthax_proc(buffer, bufsize);
    }
}
