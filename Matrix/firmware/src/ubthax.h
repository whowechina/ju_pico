/*
 * Interfacing Jubeathax (https://github.com/CrazyRedMachine/jubeathax)
 * WHowe <github.com/whowechina>
 */

#ifndef UBTHAX_H_
#define UBTHAX_H_

typedef enum {
    UBT_IDLE,
    UBT_STARTING,
    UBT_INGAME,
    UBT_RESULT,
} ubt_phase_t;

ubt_phase_t ubthax_get_phase();
void ubthax_init();
void ubthax_proc(const uint8_t *buf, uint8_t len);
void ubthax_update();

#endif
