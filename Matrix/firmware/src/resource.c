#include <pico/stdlib.h>
#include <stdint.h>

#include "marker.h"
#include "font.h"

#ifdef BOARD_JU_MATRIX
#include "res/marker/mk0001_cyber.h"
#include "res/marker/mk0002_flower.h"
#include "res/marker/mk0003_afro.h"
#include "res/marker/mk0004_shutter.h"
#include "res/marker/mk0005_blue_ring.h"
#include "res/marker/mk0006_fireworks.h"
#include "res/marker/mk0007_target.h"
#include "res/marker/mk0008_egg.h"
#include "res/marker/mk0009_gauge.h"
#include "res/marker/mk0010_ice.h"
#include "res/marker/mk0011_volcano.h"
#include "res/marker/mk0012_digital_block.h"
#include "res/marker/mk0013_sand_art.h"
#include "res/marker/mk0014_soap_ball.h"
#include "res/marker/mk0015_okojo.h"
#include "res/marker/mk0016_mirror_ball.h"
#include "res/marker/mk0017_ripples.h"
#include "res/marker/mk0018_torus.h"
#include "res/marker/mk0019_blur.h"
#include "res/marker/mk0020_touch.h"
#include "res/marker/mk0021_banana.h"
#include "res/marker/mk0022_fusuma.h"
#include "res/marker/mk0023_picture.h"
#include "res/marker/mk0024_yubiko.h"
#include "res/marker/mk0026_knit.h"
#include "res/marker/mk0027_concierge.h"
#include "res/marker/mk0028_jubeat-kun.h"
#include "res/marker/mk0029_color_composition.h"
#include "res/marker/mk0030_stopwatch.h"
#include "res/marker/mk0031_nengoro_neko.h"
#include "res/marker/mk0032_copious.h"
#include "res/marker/mk0033_circus.h"
#include "res/marker/mk0034_kobi_fukurou.h"
#include "res/marker/mk0035_natural_block.h"
#include "res/marker/mk0036_saucer.h"
#include "res/marker/mk0037_s-c-u.h"
#include "res/marker/mk0038_qma_sharon.h"
#include "res/marker/mk0039_prop.h"
#include "res/marker/mk0040_qubell.h"
#include "res/marker/mk0045_clan.h"
#include "res/marker/mk0046_festo.h"
#include "res/marker/mk0047_hinabitter.h"
#include "res/marker/mk0048_jubeat_2021.h"
#include "res/marker/mk0049_ave.h"
#elif defined BOARD_JU_MATRIX_PLUS
#include "res/marker_plus/mk0001_cyber.h"
#include "res/marker_plus/mk0002_flower.h"
#include "res/marker_plus/mk0003_afro.h"
#include "res/marker_plus/mk0004_shutter.h"
#include "res/marker_plus/mk0005_blue_ring.h"
#include "res/marker_plus/mk0006_fireworks.h"
#include "res/marker_plus/mk0007_target.h"
#include "res/marker_plus/mk0008_egg.h"
#include "res/marker_plus/mk0009_gauge.h"
#include "res/marker_plus/mk0010_ice.h"
#include "res/marker_plus/mk0011_volcano.h"
#include "res/marker_plus/mk0012_digital_block.h"
#include "res/marker_plus/mk0013_sand_art.h"
#include "res/marker_plus/mk0014_soap_ball.h"
#include "res/marker_plus/mk0015_okojo.h"
#include "res/marker_plus/mk0016_mirror_ball.h"
#include "res/marker_plus/mk0017_ripples.h"
#include "res/marker_plus/mk0018_torus.h"
#include "res/marker_plus/mk0019_blur.h"
#include "res/marker_plus/mk0020_touch.h"
#include "res/marker_plus/mk0021_banana.h"
#include "res/marker_plus/mk0022_fusuma.h"
#include "res/marker_plus/mk0023_picture.h"
#include "res/marker_plus/mk0024_yubiko.h"
#include "res/marker_plus/mk0026_knit.h"
#include "res/marker_plus/mk0027_concierge.h"
#include "res/marker_plus/mk0030_stopwatch.h"
#include "res/marker_plus/mk0031_nengoro_neko.h"
#include "res/marker_plus/mk0032_copious.h"
#include "res/marker_plus/mk0033_circus.h"
#include "res/marker_plus/mk0034_kobi_fukurou.h"
#include "res/marker_plus/mk0035_natural_block.h"
#include "res/marker_plus/mk0036_saucer.h"
#include "res/marker_plus/mk0038_qma_sharon.h"
#include "res/marker_plus/mk0039_prop.h"
#include "res/marker_plus/mk0040_qubell.h"
#include "res/marker_plus/mk0045_clan.h"
#include "res/marker_plus/mk0046_festo.h"
#include "res/marker_plus/mk0047_hinabitter.h"
#include "res/marker_plus/mk0048_jubeat_2021.h"
#include "res/marker_plus/mk0049_ave.h"
#endif


#include "res/font/dejavuserif_16x20_4bit.h"
#include "res/font/digit_dejavusansmono_9x12_4bit.h"

/*
#include "res/font/digit_dejavusansmono_16x20_4bit.h"
#include "res/font/digit_dejavusansmono_bold_16x20_4bit.h"
#include "res/font/digit_dejavusansmono_bold_9x12_4bit.h"
#include "res/font/digit_dincondensedbold_16x32_4bit.h"
#include "res/font/digit_farah_16x32_4bit.h"
#include "res/font/digit_noteworthy_16x32_4bit.h"
#include "res/font/digit_papyrus_15x32_4bit.h"
#include "res/font/digit_sana_17x32_4bit.h"
*/

const marker_res_t marker_lib[] = {
#ifdef BOARD_JU_MATRIX
    DEF_mk0001_cyber_c8_a8,
    DEF_mk0002_flower_c8_a8,
        // DEF_mk0003_afro_c8_a8,
    DEF_mk0004_shutter_c8_a8,
        // DEF_mk0005_blue_ring_c8_a8,
    DEF_mk0006_fireworks_c8_a8,
    DEF_mk0007_target_c8_a8,
    DEF_mk0008_egg_c8_a8,
    DEF_mk0009_gauge_c8_a8,
        // DEF_mk0010_ice_c8_a8,
        // DEF_mk0011_volcano_c8_a8,
        // DEF_mk0012_digital_block_c8_a8,
        // DEF_mk0013_sand_art_c8_a8,
    DEF_mk0014_soap_ball_c8_a8,
        // DEF_mk0015_okojo_c8_a8,
        // DEF_mk0016_mirror_ball_c8_a8,
    DEF_mk0017_ripples_c8_a8,
    DEF_mk0018_torus_c8_a8,
        // DEF_mk0019_blur_c8_a8,
    DEF_mk0020_touch_c8_a8,
    DEF_mk0021_banana_c8_a8,
        // DEF_mk0022_fusuma_c8_a8,
        // DEF_mk0023_picture_c8_a8,
    DEF_mk0024_yubiko_c8_a8,
    DEF_mk0026_knit_c8_a8,
        // DEF_mk0027_concierge_c8_a8,
        // DEF_mk0030_stopwatch_c8_a8,
        // DEF_mk0031_nengoro_neko_c8_a8,
    DEF_mk0032_copious_c8_a8,
        // DEF_mk0033_circus_c8_a8,
        // DEF_mk0034_kobi_fukurou_c8_a8,
    DEF_mk0035_natural_block_c8_a8,
        // DEF_mk0036_saucer_c8_a8,
    DEF_mk0038_qma_sharon_c8_a8,
        // DEF_mk0039_prop_c8_a8,
    DEF_mk0040_qubell_c8_a8,
    DEF_mk0045_clan_c8_a8,
    DEF_mk0046_festo_c8_a8,
        // DEF_mk0047_hinabitter_c8_a8,
    DEF_mk0048_jubeat_2021_c8_a8,
    DEF_mk0049_ave_c8_a8,
        // DEF_mk0028_jubeat_kun_c8_a8,
    DEF_mk0029_color_composition_c8_a8,
        // DEF_mk0037_s_c_u_c8_a8
#elif defined BOARD_JU_MATRIX_PLUS
    DEF_mk0001_cyber_c8_a8,
    DEF_mk0002_flower_c8_a8,
        // DEF_mk0003_afro_c8_a8,
    DEF_mk0004_shutter_c8_a8,
        // DEF_mk0005_blue_ring_c8_a8,
    // DEF_mk0006_fireworks_c8_a8,
    // DEF_mk0007_target_c8_a8,
    // DEF_mk0008_egg_c8_a8,
    // DEF_mk0009_gauge_c8_a8,
        // DEF_mk0010_ice_c8_a8,
        // DEF_mk0011_volcano_c8_a8,
        // DEF_mk0012_digital_block_c8_a8,
        // DEF_mk0013_sand_art_c8_a8,
    // DEF_mk0014_soap_ball_c8_a8,
        // DEF_mk0015_okojo_c8_a8,
        // DEF_mk0016_mirror_ball_c8_a8,
    // DEF_mk0017_ripples_c8_a8,
    // DEF_mk0018_torus_c8_a8,
        // DEF_mk0019_blur_c8_a8,
    // DEF_mk0020_touch_c8_a8,
    // DEF_mk0021_banana_c8_a8,
        // DEF_mk0022_fusuma_c8_a8,
        // DEF_mk0023_picture_c8_a8,
    // DEF_mk0024_yubiko_c8_a8,
    // DEF_mk0026_knit_c8_a8,
        // DEF_mk0027_concierge_c8_a8,
        // DEF_mk0030_stopwatch_c8_a8,
        // DEF_mk0031_nengoro_neko_c8_a8,
    // DEF_mk0032_copious_c8_a8,
        // DEF_mk0033_circus_c8_a8,
        // DEF_mk0034_kobi_fukurou_c8_a8,
    // DEF_mk0035_natural_block_c8_a8,
        // DEF_mk0036_saucer_c8_a8,
    // DEF_mk0038_qma_sharon_c8_a8,
        // DEF_mk0039_prop_c8_a8,
    // DEF_mk0040_qubell_c8_a8,
    // DEF_mk0045_clan_c8_a8,
    DEF_mk0046_festo_c8_a8,
        // DEF_mk0047_hinabitter_c8_a8,
    DEF_mk0048_jubeat_2021_c8_a8,
    DEF_mk0049_ave_c8_a8,
        // DEF_mk0028_jubeat_kun_c8_a8,
#endif
};

const int marker_count = count_of(marker_lib);

const font_t font_lib[] = {
    DEF_digit_font_dejavuserif_16x20_4bit,
    DEF_digit_font_dejavusansmono_9x12_4bit,
/*    DEF_digit_font_dejavusansmono_16x20_4bit,
    DEF_digit_font_dejavusansmono_bold_16x20_4bit,
    DEF_digit_font_dejavusansmono_bold_9x12_4bit,
    DEF_digit_font_dincondensedbold_16x32_4bit,
    DEF_digit_font_farah_16x32_4bit,
    DEF_digit_font_noteworthy_16x32_4bit,
    DEF_digit_font_papyrus_15x32_4bit,
    DEF_digit_font_sana_17x32_4bit,
*/
};

const int font_count = count_of(font_lib);
