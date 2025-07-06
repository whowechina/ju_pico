/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hub75.pio.h"

#include "board_defs.h"

#define WIDTH 64
#define HEIGHT 64

#define HUB75_DATA_PIN(n) (HUB75_DATA_BASE + (n))

uint16_t hub75_width()
{
    return WIDTH;
}

uint16_t hub75_height()
{
    return HEIGHT;
}

const bool clk_polarity = 1;
const bool stb_polarity = 1;

static void fm6126a_write_register(uint16_t value, uint8_t position)
{
    gpio_put(HUB75_CLK_PIN, !clk_polarity);
    gpio_put(HUB75_STROBE_PIN, !stb_polarity);

    uint8_t threshold = WIDTH - position;
    for(int i = 0; i < WIDTH; i++) {
        int j = i % 16;
        bool b = value & (1 << j);

        gpio_put(HUB75_DATA_PIN(0), b);
        gpio_put(HUB75_DATA_PIN(1), b);
        gpio_put(HUB75_DATA_PIN(2), b);
        gpio_put(HUB75_DATA_PIN(3), b);
        gpio_put(HUB75_DATA_PIN(4), b);
        gpio_put(HUB75_DATA_PIN(5), b);

        // Assert strobe/latch if i > threshold
        // This somehow indicates to the FM6126A which register we want to write :|
        gpio_put(HUB75_STROBE_PIN, i > threshold);
        gpio_put(HUB75_CLK_PIN, clk_polarity);
        sleep_us(10);
        gpio_put(HUB75_CLK_PIN, !clk_polarity);
    }
}

static void fm6126a_setup()
{
    // R0, G0, B0, R1, G1, B1
    for (int i = 0; i < 6; i++) {
        gpio_init(HUB75_DATA_PIN(i));
        gpio_set_dir(HUB75_DATA_PIN(i), GPIO_OUT);
    }

    gpio_init(HUB75_CLK_PIN);
    gpio_set_dir(HUB75_CLK_PIN, GPIO_OUT);
    gpio_init(HUB75_STROBE_PIN);
    gpio_set_dir(HUB75_STROBE_PIN, GPIO_OUT);
    gpio_init(HUB75_OEN_PIN);
    gpio_set_dir(HUB75_OEN_PIN, GPIO_OUT);

    fm6126a_write_register(0b1111111111111110, 12);
    fm6126a_write_register(0b0000001000000000, 13);
}

static struct {
    PIO pio;
    uint sm_data;
    uint sm_row;
    uint data_prog;
    uint row_prog;
} ctx = {
    .pio = pio0,
    .sm_data = 0,
    .sm_row = 1,
};

void hub75_init(bool fm6126)
{
    if (fm6126) {
        fm6126a_setup();
    }

    ctx.data_prog = pio_add_program(ctx.pio, &hub75_data_rgb888_program);
    ctx.row_prog = pio_add_program(ctx.pio, &hub75_row_program);
    hub75_data_rgb888_program_init(ctx.pio, ctx.sm_data, ctx.data_prog, HUB75_DATA_BASE, HUB75_CLK_PIN);
    hub75_row_program_init(ctx.pio, ctx.sm_row, ctx.row_prog, HUB75_ROWSEL_BASE, 5, HUB75_STROBE_PIN);
}

uint32_t canvas[HEIGHT][WIDTH] = {0};

static inline uint32_t gamma_correct_565_888(uint16_t pix)
{
    uint32_t r = pix & 0xf800u;
    //r *= r;
    uint32_t g = pix & 0x07e0u;
    //g *= g;
    uint32_t b = pix & 0x001fu;
    //b *= b;
    return (b >> 2 << 16) | (g >> 14 << 8) | (r >> 24 << 0);
}

static inline uint32_t gamma_correct(uint32_t pix)
{
    uint32_t r = (pix >> 16) & 0xff;
    uint32_t g = (pix >> 8) & 0xff;
    uint32_t b = pix & 0xff;
    r *= r;
    g *= g;
    b *= b;
    return (r >> 8) << 16 | (g >> 8) << 8 | (b >> 8);
}

void hub75_update()
{
    const uint row_block = (1 << HUB75_ROWSEL_N_PINS);
    static uint32_t gc_row[2][WIDTH];
    for (int rowsel = 0; rowsel < row_block; ++rowsel) {
        for (int x = 0; x < WIDTH; ++x) {
            gc_row[0][x] = gamma_correct(canvas[rowsel][x]);
            gc_row[1][x] = gamma_correct(canvas[rowsel + row_block][x]);
        }
        for (int bit = 0; bit < 8; ++bit) {
            hub75_data_rgb888_set_shift(ctx.pio, ctx.sm_data, ctx.data_prog, bit);
            for (int x = 0; x < WIDTH; ++x) {
                pio_sm_put_blocking(ctx.pio, ctx.sm_data, gc_row[0][x]);
                pio_sm_put_blocking(ctx.pio, ctx.sm_data, gc_row[1][x]);
            }
            // Dummy pixel per lane
            pio_sm_put_blocking(ctx.pio, ctx.sm_data, 0);
            pio_sm_put_blocking(ctx.pio, ctx.sm_data, 0);
            // SM is finished when it stalls on empty TX FIFO
            hub75_wait_tx_stall(ctx.pio, ctx.sm_data);
            // Also check that previous OEn pulse is finished, else things can get out of sequence
            hub75_wait_tx_stall(ctx.pio, ctx.sm_row);

            // Latch row data, pulse output enable for new row.
            pio_sm_put_blocking(ctx.pio, ctx.sm_row, rowsel | (100u * (1u << bit) << 5));
        }
    }
}
