/*
 * Ju Controller Buttons
 * WHowe <github.com/whowechina>
 * 
 */

#include "button.h"

#include <stdint.h>
#include <stdbool.h>

#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"

#include "config.h"
#include "board_defs.h"

static const uint8_t gpios[] = BUTTON_DEF;

#define BUTTON_NUM (sizeof(gpios))

static bool sw_val[BUTTON_NUM]; /* true if pressed */
static uint64_t sw_freeze_time[BUTTON_NUM];

void button_init()
{
    for (int i = 0; i < BUTTON_NUM; i++)
    {
        sw_val[i] = false;
        sw_freeze_time[i] = 0;
        uint8_t gpio = gpios[i];
        gpio_init(gpio);
        gpio_set_function(gpio, GPIO_FUNC_SIO);
        gpio_set_dir(gpio, GPIO_IN);
        gpio_pull_up(gpio);
    }
}

static inline bool button_pressed(int id)
{
    bool reading = gpio_get(gpios[id]);
    return !reading;
}

bool button_is_stuck()
{
    for (int i = 0; i < BUTTON_NUM; i++) {
        if (button_pressed(i)) {
            return true;
        }
    }
    return false;
}

uint8_t button_num()
{
    return BUTTON_NUM;
}

static uint32_t button_reading;

/* If a switch flips, it freezes for a while */
#define DEBOUNCE_FREEZE_TIME_US 3000
void button_update()
{
    uint64_t now = time_us_64();
    uint32_t buttons = 0;

    for (int i = BUTTON_NUM - 1; i >= 0; i--) {
        bool sw_pressed = button_pressed(i);
        
        if (now >= sw_freeze_time[i]) {
            if (sw_pressed != sw_val[i]) {
                sw_val[i] = sw_pressed;
                sw_freeze_time[i] = now + DEBOUNCE_FREEZE_TIME_US;
            }
        }

        buttons <<= 1;
        if (sw_val[i]) {
            buttons |= 1;
        }
    }

    button_reading = buttons;
}

uint32_t button_read()
{
    return button_reading;
}
