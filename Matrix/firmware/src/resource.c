#include <pico/stdlib.h>
#include <stdint.h>

#include "marker.h"
#include "font.h"

// Define DEBUG to include less resource, so to reduce the build time
//#define DEBUG

#ifdef BOARD_JU_MATRIX
#include "res/marker13/mk0001_cyber.h"
#include "res/marker13/mk0002_flower.h"
#include "res/marker13/mk0003_afro.h"
#include "res/marker13/mk0004_shutter.h"
#include "res/marker13/mk0005_blue_ring.h"
#include "res/marker13/mk0006_fireworks.h"
#include "res/marker13/mk0007_target.h"
#include "res/marker13/mk0008_egg.h"
#include "res/marker13/mk0009_gauge.h"
#include "res/marker13/mk0010_ice.h"
#include "res/marker13/mk0011_volcano.h"
#include "res/marker13/mk0012_digital_block.h"
#include "res/marker13/mk0013_sand_art.h"
#include "res/marker13/mk0014_soap_ball.h"
#include "res/marker13/mk0015_okojo.h"
#include "res/marker13/mk0016_mirror_ball.h"
#include "res/marker13/mk0017_ripples.h"
#include "res/marker13/mk0018_torus.h"
#include "res/marker13/mk0019_blur.h"
#include "res/marker13/mk0020_touch.h"
#include "res/marker13/mk0021_banana.h"
#include "res/marker13/mk0022_fusuma.h"
#include "res/marker13/mk0023_picture.h"
#include "res/marker13/mk0024_yubiko.h"
#include "res/marker13/mk0026_knit.h"
#include "res/marker13/mk0027_concierge.h"
#include "res/marker13/mk0028_jubeat-kun.h"
#include "res/marker13/mk0029_color_composition.h"
#include "res/marker13/mk0030_stopwatch.h"
#include "res/marker13/mk0031_nengoro_neko.h"
#include "res/marker13/mk0032_copious.h"
#include "res/marker13/mk0033_circus.h"
#include "res/marker13/mk0034_kobi_fukurou.h"
#include "res/marker13/mk0035_natural_block.h"
#include "res/marker13/mk0036_saucer.h"
#include "res/marker13/mk0037_s-c-u.h"
#include "res/marker13/mk0038_qma_sharon.h"
#include "res/marker13/mk0039_prop.h"
#include "res/marker13/mk0040_qubell.h"
#include "res/marker13/mk0045_clan.h"
#include "res/marker13/mk0046_festo.h"
#include "res/marker13/mk0047_hinabitter.h"
#include "res/marker13/mk0048_jubeat_2021.h"
#include "res/marker13/mk0049_ave.h"
#include "res/marker13/trail13.h"

#elif defined BOARD_JU_MATRIX_PLUS
#include "res/marker23/mk0001_cyber.h"
#include "res/marker23/mk0002_flower.h"
#include "res/marker23/mk0003_afro.h"
#include "res/marker23/mk0004_shutter.h"
#include "res/marker23/mk0005_blue_ring.h"
#include "res/marker23/mk0006_fireworks.h"
#include "res/marker23/mk0007_target.h"
#include "res/marker23/mk0008_egg.h"
#include "res/marker23/mk0009_gauge.h"
#include "res/marker23/mk0010_ice.h"
#include "res/marker23/mk0011_volcano.h"
#include "res/marker23/mk0012_digital_block.h"
#include "res/marker23/mk0013_sand_art.h"
#include "res/marker23/mk0014_soap_ball.h"
#include "res/marker23/mk0015_okojo.h"
#include "res/marker23/mk0016_mirror_ball.h"
#include "res/marker23/mk0017_ripples.h"
#include "res/marker23/mk0018_torus.h"
#include "res/marker23/mk0019_blur.h"
#include "res/marker23/mk0020_touch.h"
#include "res/marker23/mk0021_banana.h"
#include "res/marker23/mk0022_fusuma.h"
#include "res/marker23/mk0023_picture.h"
#include "res/marker23/mk0024_yubiko.h"
#include "res/marker23/mk0026_knit.h"
#include "res/marker23/mk0027_concierge.h"
#include "res/marker23/mk0030_stopwatch.h"
#include "res/marker23/mk0031_nengoro_neko.h"
#include "res/marker23/mk0032_copious.h"
#include "res/marker23/mk0033_circus.h"
#include "res/marker23/mk0034_kobi_fukurou.h"
#include "res/marker23/mk0035_natural_block.h"
#include "res/marker23/mk0036_saucer.h"
#include "res/marker23/mk0038_qma_sharon.h"
#include "res/marker23/mk0039_prop.h"
#include "res/marker23/mk0040_qubell.h"
#include "res/marker23/mk0045_clan.h"
#include "res/marker23/mk0046_festo.h"
#include "res/marker23/mk0047_hinabitter.h"
#include "res/marker23/mk0048_jubeat_2021.h"
#include "res/marker23/mk0049_ave.h"
#include "res/marker23/trail23.h"

#elif defined BOARD_JU_MATRIX_PLUS_CROP
#include "res/marker27/mk0001_cyber.h"
#include "res/marker27/mk0002_flower.h"
#include "res/marker27/mk0003_afro.h"
#include "res/marker27/mk0004_shutter.h"
#include "res/marker27/mk0005_blue_ring.h"
#include "res/marker27/mk0006_fireworks.h"
#include "res/marker27/mk0007_target.h"
#include "res/marker27/mk0008_egg.h"
#include "res/marker27/mk0009_gauge.h"
#include "res/marker27/mk0010_ice.h"
#include "res/marker27/mk0011_volcano.h"
#include "res/marker27/mk0012_digital_block.h"
#include "res/marker27/mk0013_sand_art.h"
#include "res/marker27/mk0014_soap_ball.h"
#include "res/marker27/mk0015_okojo.h"
#include "res/marker27/mk0016_mirror_ball.h"
#include "res/marker27/mk0017_ripples.h"
#include "res/marker27/mk0018_torus.h"
#include "res/marker27/mk0019_blur.h"
#include "res/marker27/mk0020_touch.h"
#include "res/marker27/mk0021_banana.h"
#include "res/marker27/mk0022_fusuma.h"
#include "res/marker27/mk0023_picture.h"
#include "res/marker27/mk0024_yubiko.h"
#include "res/marker27/mk0026_knit.h"
#include "res/marker27/mk0027_concierge.h"
#include "res/marker27/mk0030_stopwatch.h"
#include "res/marker27/mk0031_nengoro_neko.h"
#include "res/marker27/mk0032_copious.h"
#include "res/marker27/mk0033_circus.h"
#include "res/marker27/mk0034_kobi_fukurou.h"
#include "res/marker27/mk0035_natural_block.h"
#include "res/marker27/mk0036_saucer.h"
#include "res/marker27/mk0038_qma_sharon.h"
#include "res/marker27/mk0039_prop.h"
#include "res/marker27/mk0040_qubell.h"
#include "res/marker27/mk0045_clan.h"
#include "res/marker27/mk0046_festo.h"
#include "res/marker27/mk0047_hinabitter.h"
#include "res/marker27/mk0048_jubeat_2021.h"
#include "res/marker27/mk0049_ave.h"
#include "res/marker27/trail27.h"

#endif

#include "res/font/donegal1.h"
#include "res/font/font_conthrax.h"
#include "res/font/font_dejavu.h"
#include "res/font/font_ltsaeada.h"
#include "res/font/font_upheaval.h"
#include "res/font/font_satisfy.h"
#include "res/font/font_hack.h"

#ifdef BOARD_JU_MATRIX
const marker_res_t marker_lib[] = {
    DEF_mk0001_cyber_c8_a8,
#ifndef DEBUG
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
#endif
};

const trail_res_t trail_res = DEF_trail13_c4_a4;

#elif defined BOARD_JU_MATRIX_PLUS
const marker_res_t marker_lib[] = {
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
    // DEF_mk0049_ave_c8_a8,
        // DEF_mk0028_jubeat_kun_c8_a8,
};

const trail_res_t trail_res = DEF_trail23_c4_a4;

#elif defined BOARD_JU_MATRIX_PLUS_CROP
const marker_res_t marker_lib[] = {
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
};

const trail_res_t trail_res = DEF_trail27_c4_a4;

#endif

static_assert(count_of(marker_lib) > 0);
const unsigned int marker_count = count_of(marker_lib);


const lv_font_t font_lib[] = {
#ifdef BOARD_JU_MATRIX
    donegal1_32,
    lv_lts16,
    lv_lts14,
    lv_upheaval,
    lv_satisfy,
    lv_hack16,
#elif defined BOARD_JU_MATRIX_PLUS || defined BOARD_JU_MATRIX_PLUS_CROP
    donegal1_64,
    lv_lts20,
    lv_lts20,
    lv_upheaval,
    lv_satisfy,
    lv_hack16
#endif
};

static_assert(count_of(font_lib) > 0);
const unsigned int font_count = count_of(font_lib);

