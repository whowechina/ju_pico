#include <stdint.h>
#include <stdbool.h>

#include "board_defs.h"

#include "tusb.h"
#include "usb_descriptors.h"
#include "button.h"
#include "config.h"
#include "hid.h"

struct __attribute__((packed)) {
    uint16_t buttons;
    uint8_t axis[2];
} hid_report = {0};

static void report_usb_hid()
{
    if (tud_hid_ready()) {
        hid_report.buttons = button_read() & 0xffff;
        hid_report.axis[0] = 0;
        hid_report.axis[1] = 0;
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
