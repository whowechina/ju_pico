/*
 * Interfacing Jubeathax (https://github.com/CrazyRedMachine/jubeathax)
 * WHowe <github.com/whowechina>
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "pico/time.h"
#include "pico/util/queue.h"

#include "grid.h"
#include "score.h"
#include "ubthax.h"

/* The following definitions are from CrazyRedMachine's JubeatHax.
   I'm not using submodule to include jubeathax for simplicity. */

#define PRESSED (1<<5)

enum cell_state_e {
	/* PENDING values: coincide with internal judgement states for convenience (meant for animating cells while waiting for press) */
	APPROACH                = 0,                 /* cell is active but not in timing window yet */
	MISS                    = 1,                 /* button wasn't pressed on time */
	POOR                    = 2,                 /* button would be very early or very late (miss "good" windows, [-155 ; 85] ) */
	GOOD                    = 3,                 /* button would be early or late (blue "good" window, [-48 ; 48]) */
	GREAT                   = 4,                 /* button would be slightly early or late (green "good" window, [-24 ; 24]) */
	PERFECT                 = 5,                 /* button would be on time! ("perfect" window, [-12 ; 12]) */

	/* "Pressed" values: internal judgement states combined with the PRESSED flag (meant to handle button presses) */
    PRESSED_MISS            = PRESSED|MISS,    /* button was pressed but not on time */
    PRESSED_POOR            = PRESSED|POOR,      /* button was pressed very early or very late */
	PRESSED_GOOD            = PRESSED|GOOD,      /* button was pressed early or late (blue "good" window) */
	PRESSED_GREAT           = PRESSED|GREAT,     /* button was pressed slightly early or late (green "good" window) */
	PRESSED_PERFECT         = PRESSED|PERFECT,   /* button was pressed on time ("perfect" window) */

	/* Long note trail control (meant to handle long notes in combination with the PRESSED states) */
	LONG_TRAIL_DRAW_FIRST   = 128,               /* a long note trail has appeared (sent only the first time) */
	LONG_TRAIL_DRAW_CONT    = 129,               /* a long note trail has appeared (sent as long as the event is still part of the current chart chunk) */
	LONG_TRAIL_UPDATE       = 130,               /* update long note trail while holding the button */
	LONG_NOTE_RELEASE_FIRST = 131,               /* the long note has been released (sent only the first time) */
	LONG_NOTE_RELEASE_CONT  = 132,               /* the long note has been released (sent as long as the event is still part of the current chart chunk) */
	LONG_NOTE_MISS_FIRST    = 133,               /* the long note wasn't pressed on time (sent only the first time) */
	LONG_NOTE_MISS_CONT     = 134,               /* the long note wasn't pressed on time (sent as long as the event is still part of the current chart chunk) */

	/* other board states */
	LEAVE_RESULT_SCREEN     = 247,               /* game leaves result screen */
	PREVIEW                 = 248,               /* cell will be part of the starting notes (as displayed ingame before the song starts) */
	FINAL_SCORE             = 249,               /* final updated score (including end of song bonus) */
	FULL_COMBO              = 250,               /* the song just ended with a FULL COMBO clear */
	EXCELLENT               = 251,               /* the song just ended with an EXCELLENT clear */
	FULL_COMBO_ANIM         = 252,               /* the game triggers the FULL COMBO animation */
	EXCELLENT_ANIM          = 253,               /* the game triggers the EXCELLENT animation */

	/* Additional control states */
	INACTIVE                = 254,               /* cell has no ongoing event */
	UNCHANGED               = 255,               /* magic value to retain previous state in board_update call */
};

typedef struct __attribute__((packed)) {
    uint32_t note;
    uint32_t state;
    union {
        float ratio;
        struct {
            uint32_t score;
	        uint32_t combo;
        } score;
    };
} ubthax_data_t;

static ubt_phase_t current_phase = UBT_IDLE;
static uint64_t phase_start_time = 0;
static queue_t ubthax_event_queue;

static void marker_finish_cb(int col, int row, marker_mode_t mode)
{
    if (mode == MARKER_MISS) {
        score_set_combo(0);
    }
}

void ubthax_init()
{
    queue_init(&ubthax_event_queue, sizeof(ubthax_data_t), 32);
    grid_listen(marker_finish_cb);
}

ubt_phase_t ubthax_get_phase()
{
    return current_phase;
}

int ubthax_get_phase_time_ms()
{
    return (time_us_64() - phase_start_time) / 1000;
}

static uint64_t last_io_time = 0;

void ubthax_proc(const uint8_t *buf, uint8_t len)
{
    if (len < sizeof(ubthax_data_t)) {
        return;
    }

    last_io_time = time_us_64();

    ubthax_data_t *data = (ubthax_data_t *)buf;
    if (data->state == INACTIVE) {
        return;
    }
   
    if (!queue_try_add(&ubthax_event_queue, data)) {
        printf("Event queue full, dropping event\n");
    }
}

static void set_phase(ubt_phase_t phase)
{
    if (current_phase == phase) {
        return;
    }

    switch (phase) {
        case UBT_IDLE:
            break;
        case UBT_STARTING:
            score_set_combo(0);
            score_set_score(0);
            grid_preview_reset();
            break;
        case UBT_INGAME:
            break;
        case UBT_RESULT:
            score_set_combo(0);
            break;
    }

    current_phase = phase;
    phase_start_time = time_us_64();
}

static void process_ubthax_event(const ubthax_data_t *data)
{
    switch (data->state) {
        case PREVIEW:
            set_phase(UBT_STARTING);
            break;
        case APPROACH:
        case PRESSED_MISS:
        case PRESSED_GOOD:
        case PRESSED_GREAT:
        case PRESSED_PERFECT:
        case LONG_TRAIL_DRAW_FIRST:
        case LONG_TRAIL_DRAW_CONT:
        case LONG_TRAIL_UPDATE:
        case LONG_NOTE_RELEASE_FIRST:
        case LONG_NOTE_RELEASE_CONT:
        case LONG_NOTE_MISS_FIRST:
        case LONG_NOTE_MISS_CONT:
            set_phase(UBT_INGAME);
            break;
        case FINAL_SCORE:
        case FULL_COMBO:
        case EXCELLENT:
        case FULL_COMBO_ANIM:
        case EXCELLENT_ANIM:
            set_phase(UBT_RESULT);
            break;
        case LEAVE_RESULT_SCREEN:
            set_phase(UBT_IDLE);
            break;
    }

    const char *long_names[] = {
        "first", "cont", "update", "release_first", "release_cont", "miss_first", "miss_cont"
    };

    int col = data->note % 4;
    int row = data->note / 4;
    switch (data->state) {
        case PREVIEW:
            grid_preview(col, row);
            return;
        case APPROACH:
            grid_start(col, row, false);
            return;
        case MISS:
        case POOR:
        case GOOD:
        case GREAT:
        case PERFECT:
            return;
        case PRESSED_MISS:
            grid_judge(col, row, MARKER_MISS);
            score_set_score(data->score.score);
            score_set_combo(data->score.combo);
            return;           
        case PRESSED_POOR:
            grid_judge(col, row, MARKER_POOR);
            score_set_score(data->score.score);
            score_set_combo(data->score.combo);
            return;
        case PRESSED_GOOD:
            grid_judge(col, row, MARKER_GOOD);
            score_set_score(data->score.score);
            score_set_combo(data->score.combo);
            return;
        case PRESSED_GREAT:
            grid_judge(col, row, MARKER_GREAT);
            score_set_score(data->score.score);
            score_set_combo(data->score.combo);
            return;
        case PRESSED_PERFECT:
            grid_judge(col, row, MARKER_PERFECT);
            score_set_combo(data->score.combo);
            score_set_score(data->score.score);
            return;
        case LONG_TRAIL_DRAW_FIRST:
        case LONG_TRAIL_DRAW_CONT:
        case LONG_TRAIL_UPDATE:
        case LONG_NOTE_RELEASE_FIRST:
        case LONG_NOTE_RELEASE_CONT:
        case LONG_NOTE_MISS_FIRST:
        case LONG_NOTE_MISS_CONT: 
            printf("L_%s", long_names[data->state - LONG_TRAIL_DRAW_FIRST]);
            printf(" %d %d %f\n", col, row, data->ratio);
            return;
        case FINAL_SCORE:
            score_set_final_score(data->score.score);
            return;
        case FULL_COMBO:
            score_set_fullcombo();
            return;
        case EXCELLENT:
            score_set_excellent();
            return;
        default:
            break;
    }
    printf("%3llu: %lu %lu %lu %lu\n", time_us_64() / 1000 % 1000,
            data->note, data->state, data->score.score, data->score.combo);
}

void ubthax_update()
{
    if (time_us_64() > last_io_time + 30000000) {
        current_phase = UBT_IDLE;
        grid_preview_reset();
    }

    ubthax_data_t event;
    int processed = 0;
    while ((processed < 8) && queue_try_remove(&ubthax_event_queue, &event)) {
        process_ubthax_event(&event);
        processed++;
    }
    if (processed >= 5) {
        printf("Processed %d events\n", processed);
    }
}
