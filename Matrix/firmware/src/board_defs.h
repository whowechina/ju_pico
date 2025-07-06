/*
 * Ju Controller Board Definitions
 * WHowe <github.com/whowechina>
 */

#if defined BOARD_MATRIX_PICO

//#define RGB_PIN 17
//#define RGB_ORDER GRB // or RGB

/* 4 buttons, Test, Service, Coin, Aux */
#define BUTTON_DEF { 27 }

#define HUB75_DATA_BASE 0
#define HUB75_ROWSEL_BASE 6
#define HUB75_ROWSEL_N_PINS 5
#define HUB75_CLK_PIN 11
#define HUB75_STROBE_PIN 12
#define HUB75_OEN_PIN 13

#else

#endif
