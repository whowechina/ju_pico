#include <pico/stdlib.h>
#include <stdint.h>

#include "marker.h"

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
#include "res/mk0030_stopwatch.h"
#include "res/mk0031_nengoro_neko.h"
#include "res/mk0032_copious.h"
#include "res/mk0033_circus.h"
#include "res/mk0034_kobi_fukurou.h"
#include "res/mk0035_natural_block.h"
#include "res/mk0036_saucer.h"
#include "res/mk0038_qma_sharon.h"
#include "res/mk0039_prop.h"
#include "res/mk0040_qubell.h"
#include "res/mk0045_clan.h"
#include "res/mk0046_festo.h"
#include "res/mk0047_hinabitter.h"
#include "res/mk0048_jubeat_2021.h"
#include "res/mk0049_ave.h"


const marker_t markers[] = {
    DEF_mk0001_cyber,
    DEF_mk0002_flower,
    DEF_mk0003_afro,
    DEF_mk0004_shutter,
    DEF_mk0005_blue_ring,
    DEF_mk0006_fireworks,
    DEF_mk0007_target,
    DEF_mk0008_egg,
    DEF_mk0009_gauge,
    DEF_mk0010_ice,
    DEF_mk0011_volcano,
    DEF_mk0012_digital_block,
    DEF_mk0013_sand_art,
    DEF_mk0014_soap_ball,
    DEF_mk0015_okojo,
    DEF_mk0016_mirror_ball,
    DEF_mk0017_ripples,
    DEF_mk0018_torus,
    DEF_mk0019_blur,
    DEF_mk0020_touch,
    DEF_mk0021_banana,
    DEF_mk0022_fusuma,
    DEF_mk0023_picture,
    DEF_mk0024_yubiko,
    DEF_mk0026_knit,
    DEF_mk0027_concierge,
    DEF_mk0030_stopwatch,
    DEF_mk0031_nengoro_neko,
    DEF_mk0032_copious,
    DEF_mk0033_circus,
    DEF_mk0034_kobi_fukurou,
    DEF_mk0035_natural_block,
    DEF_mk0036_saucer,
    DEF_mk0038_qma_sharon,
    DEF_mk0039_prop,
    DEF_mk0040_qubell,
    DEF_mk0045_clan,
    DEF_mk0046_festo,
    DEF_mk0047_hinabitter,
    DEF_mk0048_jubeat_2021,
    DEF_mk0049_ave,
};

const int marker_count = count_of(markers);
