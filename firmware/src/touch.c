/*
 * Chu Pico Silder Keys
 * WHowe <github.com/whowechina>
 * 
 * MPR121 CapSense based Keys
 */

#include "touch.h"

#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#include "bsp/board.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"

#include "board_defs.h"

#include "config.h"
#include "mpr121.h"

static const struct {
    uint8_t addr;
    uint8_t electrodes;
} sensors[] = TOUCH_SENSOR_DEF;

#define SENSOR_NUM count_of(sensors)

static_assert(SENSOR_NUM <= 4, "Should be less than 4 sensors.");

struct touch_zones {
    uint8_t sensor_id;
    uint8_t electrode;
} touch_zones[] = TOUCH_ZONE_DEF;

static_assert(count_of(touch_zones) == 16, "Should be exactly 16 sensors.");

#define ZONE_NUM 16

static uint16_t zone_touch_counts[ZONE_NUM];

static uint16_t readings[SENSOR_NUM];
static uint64_t reading_all;

void touch_init()
{
    i2c_init(TOUCH_I2C_PORT, TOUCH_I2C_FREQ);
    gpio_set_function(TOUCH_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(TOUCH_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(TOUCH_I2C_SDA);
    gpio_pull_up(TOUCH_I2C_SCL);
    touch_sensor_init();
}

void touch_sensor_init()
{
    for (int i = 0; i < SENSOR_NUM; i++) {
        mpr121_init(sensors[i].addr, sensors[i].electrodes);
    }
    touch_update_config();
}

const char *touch_key_name(unsigned key)
{
    static char name[3] = { 0 };
    if (key < 18) {
        name[0] = "ABC"[key / 8];
        name[1] = '1' + key % 8;
    } else if (key < 34) {
        name[0] = "DE"[(key - 18) / 8];
        name[1] = '1' + (key - 18) % 8;
    } else {
        return "XX";
    }
    return name;
}

int touch_key_by_name(const char *name)
{
    if (strlen(name) != 2) {
        return -1;
    }
    if (strcasecmp(name, "XX") == 0) {
        return 255;
    }

    int zone = toupper(name[0]) - 'A';
    int id = name[1] - '1';
    if ((zone < 0) || (zone > 4) || (id < 0) || (id > 7)) {
        return -1;
    }
    if ((zone == 2) && (id > 1)) {
        return -1; // C1 and C2 only
    }
    const int offsets[] = { 0, 8, 16, 18, 26 };
    return offsets[zone] + id;
}

static uint16_t touch_reading;

static void touch_stat()
{
    static uint16_t last_reading;

    uint16_t just_touched = touch_reading & ~last_reading;
    last_reading = touch_reading;

    for (int i = 0; i < ZONE_NUM; i++) {
        if (just_touched & (1ULL << i)) {
            zone_touch_counts[i]++;
        }
    }
}

static void update_reading()
{
    touch_reading = 0;
    for (int i = 0; i < ZONE_NUM; i++) {
        uint16_t sensor = touch_zones[i].sensor_id;
        uint16_t bit = touch_zones[i].electrode;
        touch_reading <<= 1;
        touch_reading |= (readings[sensor] >> bit) & 1;
    }
}

void touch_update()
{
    reading_all = 0;
    for (int i = 0; i < SENSOR_NUM; i++) {
        readings[i] = mpr121_touched(sensors[i].addr) & 0x0fff;
        reading_all |= readings[i];
        reading_all <<= 16;
    }

    update_reading();
    touch_stat();
}

static bool sensor_ok[SENSOR_NUM];
bool touch_sensor_ok(unsigned i)
{
    if (i >= SENSOR_NUM) {
        return false;
    }
    return sensor_ok[i];
}

uint16_t touch_zone_num()
{
    return ZONE_NUM;
}

const uint16_t *touch_raw()
{
    static uint16_t readout[48] = {0}; // maximum 4 sensors * 12 electrodes
    uint16_t new_readings[48];
    uint16_t *buf = new_readings;
    for (int i = 0; i < SENSOR_NUM; i++) {
        mpr121_raw(sensors[i].addr, buf, sensors[i].electrodes);
        buf += sensors[i].electrodes;
    }
    memcpy(readout, new_readings, sizeof(readout));
    return readout;
}

bool touch_touched(unsigned key)
{
    if (key >= ZONE_NUM) {
        return 0;
    }
    return touch_reading & (1ULL << key);
}

uint16_t touch_read()
{
    return touch_reading;
}

unsigned touch_count(unsigned key)
{
    if (key >= ZONE_NUM) {
        return 0;
    }
    return zone_touch_counts[key];
}

void touch_reset_stat()
{
    memset(zone_touch_counts, 0, sizeof(zone_touch_counts));
}

void touch_update_config()
{
    for (int i = 0; i < SENSOR_NUM; i++) {
        uint8_t addr = sensors[i].addr;
        uint8_t electrodes = sensors[i].electrodes;
        mpr121_debounce(addr,
                        ju_cfg->sense.debounce_touch,
                        ju_cfg->sense.debounce_release);
        mpr121_sense(addr,
                     ju_cfg->sense.global,
                     ju_cfg->sense.zones + i * 12, electrodes);
        mpr121_filter(addr,
                      ju_cfg->sense.filter >> 6,
                      (ju_cfg->sense.filter >> 4) & 0x03,
                      ju_cfg->sense.filter & 0x07);
    }
}
