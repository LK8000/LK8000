/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

//
// This file is included by TerrainRenderer.cpp
//
// Terrain shadowing and highlight relative to type:
// shadow to blue   is 0 0 64
// highlight  to yellow is 255 255 16

#define NUM_COLOR_RAMP_LEVELS 13

#ifdef GREYSCALE
typedef Luminosity8 TerrainColor;
#else
typedef RGB8Color TerrainColor;
#endif

typedef struct _COLORRAMP
{
    int16_t height;
    TerrainColor color;
} COLORRAMP;

const COLORRAMP terrain_shadow[] = {
    {63, {60, 10, 10}}, // Low lands
    {63, {60, 10, 10}}, // LK Mountain 101016
    {63, {0, 0, 64}}, // Imhof Type 7, geomteric 1.35 9
    {63, {0, 0, 64}}, // Imhof Type 4, geomteric 1.5 8
    {63, {0, 0, 64}}, // Imhof Type 12, geomteric  1.5 8
    {63, {0, 0, 64}}, // Imhof Atlas der Schweiz
    {63, {0, 0, 64}}, // ICAO
    {63, {16, 32, 32}}, // LKoogle lowlands
    {63, {16, 32, 32}}, // LKoogle mountains
    {63, {16, 32, 32}}, // Low Alps
    {63, {16, 32, 32}}, // Alps
    {63, {16, 32, 32}}, // YouSee
    {63, {60, 60, 60}}, // HighContrast
    {63, {60, 60, 60}}, // GA Relative
    {63, {60, 10, 10}}, // LiteAlps
    {63, {60, 10, 10}}, // Low Hills
    {63, {16, 32, 32}}, // Low Alps color e-ink
    {63, {16, 32, 32}}  // Low Alps gray e-ink
};

static_assert(std::size(terrain_shadow) == NUMRAMPS, "invalid terrain_shadow");

const COLORRAMP terrain_highlight[] = {
    {255, {0, 0, 0}}, // Low lands
    {255, {0, 0, 0}}, // LK Mountain 101016
    {255, {0, 0, 0}}, // Imhof Type 7, geomteric 1.35 9
    {255, {0, 0, 0}}, // Imhof Type 4, geomteric 1.5 8
    {255, {0, 0, 0}}, // Imhof Type 12, geomteric  1.5 8
    {255, {0, 0, 0}}, // Imhof Atlas der Schweiz
    {255, {0, 0, 0}}, // ICAO
    {255, {0, 0, 0}}, // LKoogle lowlands
    {255, {0, 0, 0}}, // LKoogle mountains
    {255, {0, 0, 0}}, // Low Alps
    {255, {0, 0, 0}}, // Alps
    {255, {0, 0, 0}}, // YouSee
    {63, {250, 250, 250}}, // HighContrast
    {255, {0, 0, 0}}, // GA Relative
    {255, {0, 0, 0}}, // LiteAlps
    {255, {0, 0, 0}}, // Low Hills
    {255, {0, 0, 0}}, // Low Alps color e-ink
    {255, {0, 0, 0}}  // Low Alps gray e-ink
};

static_assert(std::size(terrain_highlight) == NUMRAMPS, "invalid terrain_highlight");

// LK Use shading for terrain modes
const bool terrain_doshading[] = {
    1, // Low lands
    1, // Mountain
    1, // Imhof Type 7, geomteric 1.35 9
    1, // Imhof Type 4, geomteric 1.5 8
    1, // Imhof Type 12, geomteric  1.5 8
    1, // Imhof Atlas der Schweiz
    1, // ICAO
    1, // LKoogle lowlands
    1, // LKoogle mountains
    1, // Low Alps
    1, // Alps
    1, // YouSee
    1, // HighContrast
    0, // GA Relative
    1, // LiteAlps
    1,  // Low Hills
    1, // Low Alps color e-ink
    1  // Low Alps gray e-ink
};

static_assert(std::size(terrain_doshading) == NUMRAMPS, "invalid terrain_doshading");

// LK Use minimal altitude normalizer for terrain modes
const bool terrain_minalt[NUMRAMPS] = {
    1, // Low lands
    1, // LK Mountain 101016
    1, // Imhof 7
    1, // IMhof 4
    1, // Imhof 12
    1, // Atlas
    1, // ICAO
    1, // LKoogle lowlands
    1, // LKoogle mountains
    1, // Low Alps
    1, // Alps
    0, // YouSee
    1, // HighContrast
    1, // GA Relative
    1, // Lite Alps
    1, // Low Hills
    1, // Low Alps color e-ink
    1  // Low Alps gray e-ink
};

static_assert(std::size(terrain_minalt) == NUMRAMPS, "invalid terrain_minalt");

const COLORRAMP terrain_colors[][NUM_COLOR_RAMP_LEVELS] = {
    {
        // Low lands
#ifdef DITHER
        {0, {0xff, 0xff, 0xff}},
        {250, {0xff, 0xff, 0xff}},
#else
        {0, {0x70, 0xc0, 0xa7}},
        {250, {0xca, 0xe7, 0xb9}},
#endif
        {500, {0xf4, 0xea, 0xaf}},
        {750, {0xdc, 0xb2, 0x82}},
        {1000, {0xca, 0x8e, 0x72}},
        {1250, {0xde, 0xc8, 0xbd}},
        {1500, {0xe3, 0xe4, 0xe9}},
        {1750, {0xdb, 0xd9, 0xef}},
        {2000, {0xce, 0xcd, 0xf5}},
        {2250, {0xc2, 0xc1, 0xfa}},
        {2500, {0xb7, 0xb9, 0xff}},
        {5000, {0xb7, 0xb9, 0xff}},
        {6000, {0xb7, 0xb9, 0xff}},
    },
    {
        // LK Mountain 101016
#ifdef DITHER
        {0, {0xff, 0xff, 0xff}},
        {350, {0xff, 0xff, 0xff}},
#else
        {0, {0x70, 0xc0, 0xa7}},
        {350, {0xca, 0xe7, 0xb9}},
#endif
        {700, {0xf4, 0xea, 0xaf}},
        {1050, {0xdc, 0xb2, 0x82}},
        {1400, {0xd1, 0x9a, 0x5c}},
        {1750, {0xca, 0x8e, 0x72}},
        {2100, {0x9b, 0x59, 0x3b}},
        {2450, {0xde, 0xc8, 0xbd}},
        {2800, {0xe3, 0xe4, 0xe9}},
        {3150, {0xdb, 0xd9, 0xef}},
        {3500, {0xce, 0xcd, 0xf5}},
        {3850, {0xb7, 0xb9, 0xff}},
        {7000, {0xff, 0xff, 0xff}},
    },
    {
        // Imhof Type 7, geomteric 1.35 9
        {0, {153, 178, 169}},
        {368, {180, 205, 181}},
        {496, {225, 233, 192}},
        {670, {255, 249, 196}},
        {905, {255, 249, 196}},
        {1222, {255, 219, 173}},
        {1650, {254, 170, 136}},
        {2227, {253, 107, 100}},
        {3007, {255, 255, 255}},
        {5000, {255, 255, 255}},
        {6000, {255, 255, 255}},
        {7000, {255, 255, 255}},
        {8000, {255, 255, 255}},
    },
    {
        // Imhof Type 4, geomteric 1.5 8
        {0, {175, 224, 203}},
        {264, {211, 237, 211}},
        {396, {254, 254, 234}},
        {594, {252, 243, 210}},
        {891, {237, 221, 195}},
        {1336, {221, 199, 175}},
        {2004, {215, 170, 148}},
        {3007, {255, 255, 255}},
        {4000, {255, 255, 255}},
        {5000, {255, 255, 255}},
        {6000, {255, 255, 255}},
        {7000, {255, 255, 255}},
        {8000, {255, 255, 255}},
    },
    {
        // Imhof Type 12, geomteric  1.5 8
        {0, {165, 220, 201}},
        {399, {219, 239, 212}},
        {558, {254, 253, 230}},
        {782, {254, 247, 211}},
        {1094, {254, 237, 202}},
        {1532, {254, 226, 207}},
        {2145, {254, 209, 204}},
        {3004, {255, 255, 255}},
        {4000, {255, 255, 255}},
        {5000, {255, 255, 255}},
        {6000, {255, 255, 255}},
        {7000, {255, 255, 255}},
        {8000, {255, 255, 255}},
    },
    {
        // Imhof Atlas der Schweiz
        {0, {47, 101, 147}},
        {368, {58, 129, 152}},
        {496, {117, 148, 153}},
        {670, {155, 178, 140}},
        {905, {192, 190, 139}},
        {1222, {215, 199, 137}},
        {1650, {229, 203, 171}},
        {2227, {246, 206, 171}},
        {3007, {252, 246, 244}},
        {5001, {252, 246, 244}},
        {7000, {252, 246, 244}},
        {8000, {252, 246, 244}},
        {9000, {252, 246, 244}},
    },
    {
        // ICAO
        {0, {180, 205, 181}},
        {199, {180, 205, 181}},
        {200, {225, 233, 192}},
        {499, {225, 233, 192}},
        {500, {255, 249, 196}},
        {999, {255, 249, 196}},
        {1000, {255, 219, 173}},
        {1499, {255, 219, 173}},
        {1500, {254, 170, 136}},
        {1999, {254, 170, 136}},
        {2000, {253, 107, 100}},
        {2499, {253, 107, 100}},
        {2500, {255, 255, 255}},
    },
    {
        // LKoogle lowlands
#ifdef DITHER
        {0, {0xff, 0xff, 0xff}},
        {250, {0xff, 0xff, 0xff}},
#else
        {0, {222, 226, 203}},
        {250, {222, 226, 203}},
#endif
        {500, {180, 180, 180}},
        {750, {170, 170, 170}},
        {1000, {160, 160, 160}},
        {1250, {140, 140, 140}},
        {1500, {130, 130, 130}},
        {1750, {120, 120, 120}},
        {2000, {190, 190, 190}},
        {2250, {215, 215, 215}},
        {2500, {240, 240, 240}},
        {3000, {240, 240, 240}},
        {4000, {240, 240, 240}},
    },
    {
        // LKoogle mountains
#ifdef DITHER
        {0, {0xff, 0xff, 0xff}},
        {500, {0xff, 0xff, 0xff}},
#else
        {0, {222, 226, 203}},
        {500, {222, 226, 203}},
#endif
        {1000, {180, 180, 180}},
        {1500, {160, 160, 160}},
        {2000, {140, 140, 140}},
        {2500, {120, 120, 120}},
        {3000, {190, 190, 190}},
        {3500, {215, 215, 215}},
        {4000, {240, 240, 240}},
        {4500, {240, 240, 240}},
        {5000, {240, 240, 240}},
        {6000, {240, 240, 240}},
        {7000, {240, 240, 240}},
    },
    {
        // Low Alps
#ifdef DITHER
        {0, {0xff, 0xff, 0xff}},
        {250, {0xff, 0xff, 0xff}},
#else
        {0, {0x70, 0xc0, 0xa7}},
        {250, {0xca, 0xe7, 0xb9}},
#endif
        {500, {0xf4, 0xea, 0xaf}},
        {750, {0xdc, 0xb2, 0x82}},
        {1000, {0xca, 0x8e, 0x72}},
        {1250, {180, 180, 180}},
        {1500, {160, 160, 160}},
        {1750, {150, 150, 150}},
        {2000, {140, 140, 140}},
        {2250, {130, 130, 130}},
        {2500, {200, 200, 200}},
        {3000, {220, 220, 220}},
        {4000, {240, 240, 240}},
    },
    {
        // Alps
#ifdef DITHER
        {150, {0xff, 0xff, 0xff}},
        {350, {0xff, 0xff, 0xff}},
        {500, {0xff, 0xff, 0xff}},
#else
        {150, {0x70, 0xc0, 0xa7}},
        {350, {0xca, 0xe7, 0xb9}},
        {500, {0xca, 0xe7, 0xb9}},
#endif
        {650, {0xf4, 0xea, 0xaf}},
        {800, {0xf4, 0xea, 0xaf}},
        {950, {0xdc, 0xb2, 0x82}},
        {1100, {0xca, 0x8e, 0x72}},
        {1500, {175, 175, 175}},
        {1750, {165, 165, 165}},
        {2000, {155, 155, 155}},
        {2500, {235, 235, 235}},
        {3000, {245, 245, 245}},
        {4000, {255, 255, 255}},
    },
    {
        // YouSee
#ifdef DITHER
        {0, {0xff, 0xff, 0xff}},
#else
        {0, {112, 191, 170}},
#endif
        {800, {254, 255, 188}},
        {1900, {194, 135, 93}},
        {2900, {230, 230, 228}},
        {4900, {186, 185, 251}},
        {6000, {255, 255, 255}},
        {6000, {255, 255, 255}},
        {6000, {255, 255, 255}},
        {6000, {255, 255, 255}},
        {6000, {255, 255, 255}},
        {6000, {255, 255, 255}},
        {6000, {255, 255, 255}},
        {6000, {255, 255, 255}},
    },
    {
        // HighContrast
#ifdef DITHER
        {0, {0xff, 0xff, 0xff}},
#else
        {0, {235, 255, 235}},
#endif
        {100, {197, 216, 246}},
        {200, {0, 170, 0}},
        {300, {0, 128, 0}},
        {400, {0, 85, 0}},
        {500, {51, 119, 0}},
        {700, {153, 187, 0}},
        {900, {255, 255, 0}},
        {1000, {241, 227, 0}},
        {1300, {199, 142, 0}},
        {1800, {128, 0, 0}},
        {3100, {255, 255, 255}},
        {4900, {160, 191, 237}},
    },
    {
        // GA Relative
#ifdef DITHER
        {0, {0xff, 0xff, 0xff}},
#else
        {0, {227, 255, 224}},
#endif
        {50, {227, 255, 224}},
        {51, {255, 255, 0}},
        {120, {255, 255, 0}},
        {149, {255, 100, 50}},
        {150, {255, 0, 0}}, // 0m
        {300, {255, 0, 0}},
        {500, {220, 0, 0}},
        {700, {200, 0, 0}},
        {900, {180, 0, 0}},
        {1100, {150, 0, 0}},
        {1300, {120, 0, 0}},
        {3500, {100, 0, 0}},
    },
    {
        // LiteAlps
#ifdef DITHER
        {0, {0xff, 0xff, 0xff}},
        {250, {0xff, 0xff, 0xff}},
#else
        {0, {0xff, 0xff, 0xff}},
        {250, {0xca, 0xe7, 0xb9}},
#endif
        {400, {0x3d, 0xb3, 0x8b}},
        {550, {0x7f, 0xc7, 0x5c}},
        {700, {0x37, 0xa0, 0x7d}},
        {850, {0xf4, 0xea, 0xaf}},
        {1250, {0xdc, 0xb2, 0x82}},
        {1500, {0xca, 0x8e, 0x72}},
        {1750, {0xde, 0xc8, 0xbd}},
        {2000, {0xe3, 0xe4, 0xe9}},
        {2300, {0xdb, 0xd9, 0xef}},
        {3000, {0xb7, 0xb9, 0xff}},
        {6000, {0xef, 0xef, 0xff}},
    },
    {
        // Low Hills
#ifdef DITHER
        {0, {0xff, 0xff, 0xff}},
        {50, {0xff, 0xff, 0xff}},
#else
        {0, {0xff, 0xff, 0xff}},
        {25, {0xff, 0xff, 0xff}},
#endif
        {150, {0xca, 0xe7, 0xb9}},  // very light green
        {200, {0x3d, 0xb3, 0x8b}},  // darker green
        {250, {0x7f, 0xc7, 0x5c}},  // green/yellow
        {300, {0x37, 0xa0, 0x7d}},  // green
        {350, {0xf4, 0xea, 0xaf}},  // yellow
        {400, {0xdc, 0xb2, 0x82}},  // yellow brown
        {550, {0x99, 0x33, 0x00}},  // brown
        {650, {0xca, 0x8e, 0x72}},  // light brown
        {750, {0xde, 0xc8, 0xbd}},  // very light brown
        {1000, {0xe3, 0xe4, 0xe9}}, // light ice
        {1000, {0xe3, 0xe4, 0xe9}}, // light ice
    },
    {
        // Low Alps color e-ink
        {0, {0xff, 0xff, 0xff}},
        {250, {0xff, 0xff, 0xff}},
        {500, {0x70, 0xc0, 0xa7}},
        {750, {0xca, 0xe7, 0xb9}},
        {1000, {0xf4, 0xea, 0xaf}},
        {1250, {0xdc, 0xb2, 0x82}},
        {1500, {0xca, 0x8e, 0x72}},
        {1750, {180, 180, 180}},
        {2000, {140, 140, 140}},
        {2250, {130, 130, 130}},
        {2500, {200, 200, 200}},
        {3000, {220, 220, 220}},
        {4000, {240, 240, 240}},
    },
    {
        // Low Alps gray e-ink
        {0, {0xff, 0xff, 0xff}},
        {250, {0xff, 0xff, 0xff}},
        {500, {0xf4, 0xea, 0xaf}},
        {750, {0xdc, 0xb2, 0x82}},
        {1000, {0xca, 0x8e, 0x72}},
        {1250, {180, 180, 180}},
        {1500, {160, 160, 160}},
        {1750, {150, 150, 150}},
        {2000, {140, 140, 140}},
        {2250, {130, 130, 130}},
        {2500, {200, 200, 200}},
        {3000, {220, 220, 220}},
        {4000, {240, 240, 240}},
    },
};

static_assert(std::size(terrain_colors) == NUMRAMPS, "invalid terrain_colors");
