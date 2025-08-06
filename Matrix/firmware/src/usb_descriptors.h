#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "common/tusb_common.h"
#include "device/usbd.h"

enum {
    REPORT_ID_JOYSTICK = 1,
    REPORT_ID_JUBEATHAX = 7,
};

#define MATRIX_PICO_REPORT_DESC_JOYSTICK                                     \
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),                                  \
    HID_USAGE(HID_USAGE_DESKTOP_GAMEPAD),                                    \
    HID_COLLECTION(HID_COLLECTION_APPLICATION),                              \
        HID_REPORT_ID(REPORT_ID_JOYSTICK)                                    \
        HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON),                               \
        HID_USAGE_MIN(1), HID_USAGE_MAX(8),                                  \
        HID_LOGICAL_MIN(0), HID_LOGICAL_MAX(1),                              \
        HID_REPORT_COUNT(8), HID_REPORT_SIZE(1),                             \
        HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),                   \
                                                                             \
        HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP),                              \
        HID_REPORT_ID(REPORT_ID_JUBEATHAX)                                   \
        HID_USAGE(0xc5),                                                     \
        HID_USAGE_MIN(0x00), HID_USAGE_MAX_N(255, 2),                        \
        HID_REPORT_COUNT(19), HID_REPORT_SIZE(8),                            \
        HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),                 \
    HID_COLLECTION_END

#endif /* USB_DESCRIPTORS_H_ */
