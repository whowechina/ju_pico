#include <pico/stdlib.h>
#include <stdint.h>

#include "marker.h"
#include "font.h"

#include "res/mk0001_cyber.h"
#include "res/mk0002_flower.h"
#include "res/mk0003_afro.h"
#include "res/mk0004_shutter.h"
#include "res/mk0005_blue_ring.h"
#include "res/mk0006_fireworks.h"
#include "res/mk0007_target.h"
#include "res/mk0008_egg.h"
#include "res/mk0009_gauge.h"
#include "res/mk0010_ice.h"
#include "res/mk0011_volcano.h"
#include "res/mk0012_digital_block.h"
#include "res/mk0013_sand_art.h"
#include "res/mk0014_soap_ball.h"
#include "res/mk0015_okojo.h"
#include "res/mk0016_mirror_ball.h"
#include "res/mk0017_ripples.h"
#include "res/mk0018_torus.h"
#include "res/mk0019_blur.h"
#include "res/mk0020_touch.h"
#include "res/mk0021_banana.h"
#include "res/mk0022_fusuma.h"
#include "res/mk0023_picture.h"
#include "res/mk0024_yubiko.h"
#include "res/mk0026_knit.h"
#include "res/mk0027_concierge.h"
#include "res/mk0028_jubeat-kun.h"
#include "res/mk0029_color_composition.h"
#include "res/mk0030_stopwatch.h"
#include "res/mk0031_nengoro_neko.h"
#include "res/mk0032_copious.h"
#include "res/mk0033_circus.h"
#include "res/mk0034_kobi_fukurou.h"
#include "res/mk0035_natural_block.h"
#include "res/mk0036_saucer.h"
#include "res/mk0037_s-c-u.h"
#include "res/mk0038_qma_sharon.h"
#include "res/mk0039_prop.h"
#include "res/mk0040_qubell.h"
#include "res/mk0045_clan.h"
#include "res/mk0046_festo.h"
#include "res/mk0047_hinabitter.h"
#include "res/mk0048_jubeat_2021.h"
#include "res/mk0049_ave.h"

#include "res/digit_font_dincondensedbold_16x32_4bit.h"
#include "res/digit_font_farah_16x32_4bit.h"
#include "res/digit_font_noteworthy_16x32_4bit.h"
#include "res/digit_font_papyrus_15x32_4bit.h"
#include "res/digit_font_sana_17x32_4bit.h"
#include "res/digit_font_dejavuserif_16x20_4bit.h"
#include "res/digit_font_dejavusansmono_16x20_4bit.h"
#include "res/digit_font_dejavusansmono_bold_16x20_4bit.h"

const marker_res_t marker_lib[] = {
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
};

const int marker_count = count_of(marker_lib);

const font_t font_lib[] = {
    DEF_digit_font_dincondensedbold_16x32_4bit,
    DEF_digit_font_farah_16x32_4bit,
    DEF_digit_font_noteworthy_16x32_4bit,
    DEF_digit_font_papyrus_15x32_4bit,
    DEF_digit_font_sana_17x32_4bit,
    DEF_digit_font_dejavuserif_16x20_4bit,
    DEF_digit_font_dejavusansmono_16x20_4bit,
    DEF_digit_font_dejavusansmono_bold_16x20_4bit,
};

const int font_count = count_of(font_lib);
