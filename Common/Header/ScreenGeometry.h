/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   PLEASE USE COMMENTS ALSO HERE TO DESCRIBE YOUR GLOBALS!
   YOU CAN INITIALIZE VALUES TO true,false,zero and NULL,
   or you can do it also inside Globals_Init.

*/

#ifndef SCREENGEO_H
#define SCREENGEO_H

#include "Compiler.h"
#include <type_traits>

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

/**
 * screen_scale_int and screen_scale_double need to have same value and used only by IBLSCALE() function
 *  this class exist for enforce this requirement,
 *  helper struct are made for select right value (int or double at compil time.
 */
class ScreenScale final {
public:

    template<class T, bool = std::is_integral<T>::value>
    struct helper { };

    template<class T>
    struct helper<T, true> {
        static T scale( T x )
        {
            return (x * screen_scale_int) >> radix_shift;
        }
    };

    template<class T>
    struct helper<T, false> {
        static T scale ( T x )
        {
            return x * screen_scale_double;
        }
    };

protected:

    friend void InitLKScreen();

    static void set(double scale) {
        screen_scale_double = scale;
        screen_scale_int = static_cast<int>(scale*radix);
    }

    static double get() {
        return screen_scale_double;
    }

private:
    constexpr static int radix_shift = 10;
    constexpr static int radix = 1 << radix_shift;

    static int screen_scale_int;
    static double screen_scale_double;
};


template<class T>
gcc_pure inline
T IBLSCALE(T x) {
    return ScreenScale::helper<T>::scale(x);
}

#define NIBLSCALE IBLSCALE

gcc_pure inline
int DLGSCALE(int x) {
    return IBLSCALE(x);
}

unsigned int TerrainQuantization();


#endif
