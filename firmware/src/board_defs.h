/*
 * Ju Controller Board Definitions
 * WHowe <github.com/whowechina>
 */

#if defined BOARD_JU_PICO

#define RGB_PIN 17
#define RGB_ORDER GRB // or RGB

/* 4 buttons, Test, Service, Coin, Aux */
#define BUTTON_DEF { 4, 7, 0, 3, 5, 6, 1, 2, 10, 9, 14, 13, 11, 8, 15, 12, 18, 19, 20, 21 }

#else

#endif
