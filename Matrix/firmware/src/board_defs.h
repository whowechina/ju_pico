/*
 * Ju Controller Board Definitions
 * WHowe <github.com/whowechina>
 */

#define BUTTON_DEF { 27 }

#define HUB75_DATA_BASE 0
#define HUB75_ROWSEL_BASE 6
#define HUB75_ROWSEL_N_PINS 5
#define HUB75_CLK_PIN 11
#define HUB75_STROBE_PIN 12
#define HUB75_OEN_PIN 13

#if defined BOARD_JU_MATRIX
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64
#define GRID_SIZE 13
#define GRID_GAP 4

#elif defined BOARD_JU_MATRIX_PLUS
#define PANEL_WIDTH 128
#define PANEL_HEIGHT 128
#define GRID_SIZE 28
#define GRID_GAP 6
#endif
