/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   PLEASE USE COMMENTS ALSO HERE TO DESCRIBE YOUR GLOBALS!
   YOU CAN INITIALIZE VALUES TO true,false,zero and NULL,
   or you can do it also inside Globals_Init.

*/

#ifndef SCREENGEO_H
#define SCREENGEO_H

#include "Compiler.h"

#define SCREEN_GEOMETRY_INVALID  0

//
// Each geometry has Screen0Ratio rescaled upon a single fine-tuned setting
// different among portrait and landscape orientations. This is done by
// GetScreen0Ratio() . Always remember: Screen0Ratio is a vertical scale ratio.
// Because fonts are being rescaled vertically.
//
#define SCREEN_GEOMETRY_SQUARED  1 // Unknown
#define SCREEN_GEOMETRY_43       2 // L: 640x480 (480)  P: 480x640 (640)
#define SCREEN_GEOMETRY_53       3 // L: 800x480 (480)  P: 480x800 (800)
#define SCREEN_GEOMETRY_169      4 // L: 480x272 (272)  P: 272x480 (480)
#define SCREEN_GEOMETRY_21       5 // L: 480x234 (234)  P: 234x480 (480)
//
// In case of Unknown geometry we use L: 480x272 (272)  P: 272x480 (480)
//
#define SCREEN_GEOMETRY_COUNT    6

// Many graphic functions in LK were tuned around low DPI devices, with the most tuned being 
// 480x272 at around 110 DPI. 800x480 was also managed decently at 186 DPI.
// Here we fix a reference DPI to rescale pixels. Lower value means bigger rescaling.
#define LK_REFERENCE_DPI  80

/**
 * Rescale pixel size depending on DPI. Most sizes are tuned for 110-180 dpi . We need to rescale them.
 * If unused, this function is a transparent #define RescalePixelSize(arg) arg
 * See ScreenGeometry.h
 * WARNING: use this function only after ScreenPixelRatio has been calculated by InitLKScreen().
 */
gcc_pure inline
int RescalePixelSize(int x) {
#ifdef RESCALE_PIXEL
    return (x * ScreenPixelRatio) >> 10;
#else
    return x;
#endif
}

gcc_pure inline
int IBLSCALE(int x) {
    extern int ScreenScale;
    return (x * ScreenScale) >> 10;
}

#define NIBLSCALE IBLSCALE

gcc_pure inline
int DLGSCALE(int x) {
    return IBLSCALE(x);
}


#endif
