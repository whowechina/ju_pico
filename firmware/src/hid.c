#include <stdint.h>
#include <stdbool.h>

#include "board_defs.h"

#include "tusb.h"
#include "usb_descriptors.h"
#include "button.h"
#include "touch.h"
#include "config.h"
#include "hid.h"

struct __attribute__((packed)) {
    uint16_t buttons;
    uint8_t aux;
    uint8_t axis[2];
} hid_report = {0};

/* map reflects to a 4x4 button matrix status */
static uint16_t do_rotate_90cw(uint16_t map)
{
    uint16_t ret = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (map & (1 << (i * 4 + j))) {
                ret |= 1 << (j * 4 + (3 - i));
            }
        }
    }
    return ret;
}

static uint16_t do_rotate(uint16_t map)
{
    uint16_t ret = map;
    for (int i = 0; i < ju_cfg->hid.rotate % 4; i++) {
        ret = do_rotate_90cw(ret);
    }
    return ret;
}

static void report_usb_hid()
{
    if (tud_hid_ready()) {
        hid_report.buttons = do_rotate(button_read() & 0xffff);
        hid_report.aux = button_read() >> 16;
        hid_report.axis[0] = 127;
        hid_report.axis[1] = 127;
        tud_hid_n_report(0, REPORT_ID_JOYSTICK, &hid_report, sizeof(hid_report));
    }
}

void hid_update()
{
    report_usb_hid();
}

void hid_proc(const uint8_t *data, uint8_t len)
{
}
