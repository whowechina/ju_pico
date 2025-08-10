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

#define JUDGE (1<<5)

enum cell_state_e {
	/* PENDING values: coincide with internal judgement states for convenience (meant for animating cells while waiting for press) */
	APPROACH                = 0,                 /* cell is active but not in timing window yet */
	MISS                    = 1,                 /* button wasn't pressed on time */
	POOR                    = 2,                 /* button would be very early or very late (miss "good" windows, [-155 ; 85] ) */
	GOOD                    = 3,                 /* button would be early or late (blue "good" window, [-48 ; 48]) */
	GREAT                   = 4,                 /* button would be slightly early or late (green "good" window, [-24 ; 24]) */
	PERFECT                 = 5,                 /* button would be on time! ("perfect" window, [-12 ; 12]) */

	/* "Pressed" values: internal judgement states combined with the JUDGE flag (meant to handle button presses) */
	JUDGE_DROP            = JUDGE|APPROACH,  /* button was pressed but not on time */
	JUDGE_MISS            = JUDGE|MISS,      /* button was pressed but not on time */
    JUDGE_POOR            = JUDGE|POOR,      /* button was pressed very early or very late */
	JUDGE_GOOD            = JUDGE|GOOD,      /* button was pressed early or late (blue "good" window) */
	JUDGE_GREAT           = JUDGE|GREAT,     /* button was pressed slightly early or late (green "good" window) */
	JUDGE_PERFECT         = JUDGE|PERFECT,   /* button was pressed on time ("perfect" window) */

	/* Long note trail control (meant to handle long notes in combination with the JUDGE states) */
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
    uint32_t note : 4;
    uint32_t long_dir : 2;
    uint32_t long_len : 2;
    uint32_t long_span : 24;
    uint32_t state;
    union {
        uint8_t raw[8];
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

void ubthax_init()
{
    queue_init(&ubthax_event_queue, sizeof(ubthax_data_t), 32);
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

    printf("%4ld: Phase %d -> %d\n", time_us_32() / 1000 % 10000, current_phase, phase);

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
            break;
    }

    current_phase = phase;
    phase_start_time = time_us_64();
}

static void process_ubthax_event(const ubthax_data_t *data)
{
    char info[32] = "Not Set";

    switch (data->state) {
        case PREVIEW:
            set_phase(UBT_STARTING);
            break;
        case APPROACH:
        case JUDGE_DROP:
        case JUDGE_MISS:
        case JUDGE_GOOD:
        case JUDGE_GREAT:
        case JUDGE_PERFECT:
        case LONG_TRAIL_DRAW_FIRST:
        case LONG_TRAIL_DRAW_CONT:
        case LONG_TRAIL_UPDATE:
        case LONG_NOTE_RELEASE_FIRST:
        case LONG_NOTE_RELEASE_CONT:
        case LONG_NOTE_MISS_FIRST:
        case LONG_NOTE_MISS_CONT:
            set_phase(UBT_INGAME);
            break;
        case FULL_COMBO:
        case EXCELLENT:
            break;
        case FINAL_SCORE:
        case FULL_COMBO_ANIM:
        case EXCELLENT_ANIM:
            set_phase(UBT_RESULT);
            break;
        case LEAVE_RESULT_SCREEN:
            set_phase(UBT_IDLE);
            break;
    }

    int col = data->note % 4;
    int row = data->note / 4;
    switch (data->state) {
        case PREVIEW:
            grid_preview(col, row);
            return;
        case APPROACH:
            grid_schedule(col, row, false);
            return;
        case MISS:
        case POOR:
        case GOOD:
        case GREAT:
        case PERFECT:
            return;
        case JUDGE_MISS:
            grid_judge(col, row, MARKER_MISS);
            score_set_score(data->score.score);
            score_set_combo(data->score.combo);
            sprintf(info, "miss");
            break;           
        case JUDGE_POOR:
            grid_judge(col, row, MARKER_POOR);
            score_set_score(data->score.score);
            score_set_combo(data->score.combo);
            sprintf(info, "poor");
            break;           
        case JUDGE_GOOD:
            grid_judge(col, row, MARKER_GOOD);
            score_set_score(data->score.score);
            score_set_combo(data->score.combo);
            sprintf(info, "good");
            break;           
        case JUDGE_GREAT:
            grid_judge(col, row, MARKER_GREAT);
            score_set_score(data->score.score);
            score_set_combo(data->score.combo);
            sprintf(info, "great");
            break;           
        case JUDGE_PERFECT:
            grid_judge(col, row, MARKER_PERFECT);
            score_set_combo(data->score.combo);
            score_set_score(data->score.score);
            sprintf(info, "perfect");
            break;
        case JUDGE_DROP:
            grid_judge(col, row, MARKER_PERFECT);
            score_set_combo(data->score.combo);
            score_set_score(data->score.score);
            sprintf(info, "maybe perfect");
            break;
        case LONG_TRAIL_DRAW_FIRST:
            grid_attach_trail(col, row, data->long_dir, data->long_len, data->long_span, true);
            sprintf(info, "trail span:%d", data->long_span);
            break;
        case LONG_TRAIL_DRAW_CONT:
            grid_attach_trail(col, row, data->long_dir, data->long_len, data->long_span, false);
            return;
        case LONG_TRAIL_UPDATE:
            grid_update_trail(col, row, data->ratio);
            sprintf(info, "update:%f", data->ratio);
            return;
        case LONG_NOTE_RELEASE_FIRST:
            sprintf(info, "trail end");
            grid_end_trail(col, row);
            break;
        case LONG_NOTE_RELEASE_CONT:
            grid_end_trail(col, row);
            return;
        case LONG_NOTE_MISS_FIRST:
        case LONG_NOTE_MISS_CONT:
            grid_end_trail(col, row);
            return;
        case FINAL_SCORE:
            score_set_score(data->score.score);
            printf("Final Score: %lu\n", data->score.score);
            score_end_result();
            return;
        case FULL_COMBO_ANIM:
            score_end_fullcombo();
            return;
        case EXCELLENT_ANIM:
            score_end_excellent();
            return;
        default:
            break;
    }

    printf("%5ld:%d%d %2lx %s\n", time_us_32() / 1000 % 100000,
            data->note % 4, data->note / 4, data->state, info);
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
