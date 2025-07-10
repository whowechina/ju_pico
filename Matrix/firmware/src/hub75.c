/*
   HUB75 driver
   This is modified from Pimoroni's HUB75 driver:
   https://github.com/pimoroni/pimoroni-pico
*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "pico/stdio.h"
#include "pico/time.h"

#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

#include "hub75.pio.h"

#include "board_defs.h"
#include "hub75.h"

#define WIDTH 64
#define HEIGHT 64
#define BIT_DEPTH 10

#define HUB75_DATA_PIN(n) (HUB75_DATA_BASE + (n))

uint32_t canvas[HEIGHT][WIDTH] = {0};
uint32_t back_buffer[HEIGHT / 2][WIDTH * 2] = {0};

struct {
    PIO pio;
    uint sm_data;
    uint sm_row;
    uint data_prog;
    uint row_prog;
    int dma_channel;
    bool fm6126;
    bool inverted_stb;
    uint row;
    uint bit;
    uint brightness;
    const bool clk_polarity;
    const bool stb_polarity;
    const bool oe_polarity;
    hub75_dma_complete_cb dma_complete;
} ctx = {
    .pio = pio0,
    .sm_data = 0,
    .sm_row = 1,
    .data_prog = 0,
    .row_prog = 0,
    .dma_channel = -1,
    .fm6126 = false,
    .inverted_stb = false,

    .row = 0,
    .bit = 0,
    .brightness = 18,

    .clk_polarity = 1,
    .stb_polarity = 1,
    .oe_polarity = 0
};

uint32_t hub75_hsv2rgb(uint8_t h, uint8_t s, uint8_t v)
{
    // Convert HSV to RGB
    if (s == 0) {
        return hub75_argb(0, v, v, v); // Grayscale
    }

    uint8_t region = h / 43;
    uint8_t remainder = (h - (region * 43)) * 6;

    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0: return hub75_argb(0, v, t, p);
        case 1: return hub75_argb(0, q, v, p);
        case 2: return hub75_argb(0, p, v, t);
        case 3: return hub75_argb(0, p, q, v);
        case 4: return hub75_argb(0, t, p, v);
        default: return hub75_argb(0, v, p, q);
    }
}

static void fm6126a_write_register(uint16_t value, uint8_t position)
{
    gpio_put(HUB75_CLK_PIN, !ctx.clk_polarity);
    gpio_put(HUB75_STROBE_PIN, !ctx.stb_polarity);

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
        gpio_put(HUB75_CLK_PIN, ctx.clk_polarity);
        sleep_us(10);
        gpio_put(HUB75_CLK_PIN, !ctx.clk_polarity);
    }
}

static void hub75_gpio_init()
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
}

static void fm6126a_setup()
{
    fm6126a_write_register(0b1111111111111110, 12);
    fm6126a_write_register(0b0000001000000000, 13);
}

void hub75_init(bool fm6126, bool inverted_stb)
{
    ctx.fm6126 = fm6126;
    ctx.inverted_stb = inverted_stb;
}

static void dma_complete()
{
    if (dma_channel_get_irq0_status(ctx.dma_channel)) {
        dma_channel_acknowledge_irq0(ctx.dma_channel);

        // Push out more dummy pixels to ensure complete data shift
        pio_sm_put_blocking(ctx.pio, ctx.sm_data, 0);
        pio_sm_put_blocking(ctx.pio, ctx.sm_data, 0);

        // SM is finished when it stalls on empty TX FIFO
        hub75_wait_tx_stall(ctx.pio, ctx.sm_data);

        // Check that previous OEn pulse is finished, else things WILL get out of sequence
        hub75_wait_tx_stall(ctx.pio, ctx.sm_row);

        // Latch row data, pulse output enable for new row.
        uint pwm_width = ctx.brightness << ctx.bit;
        pio_sm_put_blocking(ctx.pio, ctx.sm_row, ctx.row | (pwm_width << 5));

        ctx.row++;
        if (ctx.row == (1 << HUB75_ROWSEL_N_PINS)) {
            ctx.row = 0;
            ctx.bit++;
            if (ctx.bit == BIT_DEPTH) {
                ctx.bit = 0;
            }
        }

        if (ctx.dma_complete) {
            ctx.dma_complete(ctx.row, ctx.bit);
        }

        hub75_data_rgb888_set_shift(ctx.pio, ctx.sm_data, ctx.data_prog, ctx.bit);
        dma_channel_set_trans_count(ctx.dma_channel, WIDTH * 2, false);
        dma_channel_set_read_addr(ctx.dma_channel, &back_buffer[ctx.row], true);
    }
}

void hub75_start(hub75_dma_complete_cb cb)
{
    hub75_gpio_init();
    if (ctx.fm6126) {
        fm6126a_setup();
    }

    ctx.dma_complete = cb;

    uint latch_cycles = clock_get_hz(clk_sys) / 4000000;

    pio_sm_claim(ctx.pio, ctx.sm_data);
    pio_sm_claim(ctx.pio, ctx.sm_row);

    ctx.data_prog = pio_add_program(ctx.pio, &hub75_data_rgb888_program);
    if (ctx.inverted_stb) {
        ctx.row_prog = pio_add_program(ctx.pio, &hub75_row_inverted_program);
    } else {
        ctx.row_prog = pio_add_program(ctx.pio, &hub75_row_program);
    }

    hub75_data_rgb888_program_init(ctx.pio, ctx.sm_data, ctx.data_prog, HUB75_DATA_BASE, HUB75_CLK_PIN);
    hub75_row_program_init(ctx.pio, ctx.sm_row, ctx.row_prog, HUB75_ROWSEL_BASE, 5, HUB75_STROBE_PIN, latch_cycles);

    // Prevent flicker in Python caused by the smaller dataset just blasting through the PIO too quickly
    pio_sm_set_clkdiv(ctx.pio, ctx.sm_data, WIDTH <= 32 ? 2.0f : 1.0f);

    ctx.dma_channel = dma_claim_unused_channel(true);
    dma_channel_config config = dma_channel_get_default_config(ctx.dma_channel);
    channel_config_set_transfer_data_size(&config, DMA_SIZE_32);
    channel_config_set_bswap(&config, false);
    channel_config_set_dreq(&config, pio_get_dreq(ctx.pio, ctx.sm_data, true));
    dma_channel_configure(ctx.dma_channel, &config, &ctx.pio->txf[ctx.sm_data], NULL, 0, false);

    // Same handler for both DMA channels
    irq_add_shared_handler(DMA_IRQ_0, dma_complete, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    dma_channel_set_irq0_enabled(ctx.dma_channel, true);
    irq_set_enabled(DMA_IRQ_0, true);

    ctx.row = 0;
    ctx.bit = 0;

    hub75_data_rgb888_set_shift(ctx.pio, ctx.sm_data, ctx.data_prog, ctx.bit);
    dma_channel_set_trans_count(ctx.dma_channel, WIDTH * 2, false);
    dma_channel_set_read_addr(ctx.dma_channel, back_buffer, true);
}

void hub75_update()
{
    const uint row_block = (1 << HUB75_ROWSEL_N_PINS);
    for (int row = 0; row < row_block; ++row) {
        for (int x = 0; x < WIDTH; ++x) {
            back_buffer[row][x * 2] = canvas[row][x];
            back_buffer[row][x * 2 + 1] = canvas[row + row_block][x];
        }
    }
}


void hub75_clear()
{
    memset(canvas, 0, sizeof(canvas));
}
