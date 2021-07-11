/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
 */

#include "externs.h"
#include "LKInterface.h"
#include "DoInits.h"
#include "ScreenGeometry.h"
#include "Util/Clamp.hpp"
#ifdef ANDROID
#include "Android/Main.hpp"
#include "Android/NativeView.hpp"
#endif
#ifdef KOBO
#include "Kobo/Model.hpp"
#endif
//#ifdef WIN32
//#include <windows.h>
//#endif
#ifdef USE_X11
#include <X11/Xlib.h>
#endif

// Our reference DPI for scaling: we tuned objects for 320x240@3.5" (114dpi) and 480x272@5" (110dpi). 
// So we can use 110.
#define LK_REFERENCE_DPI 110

// The default size of lcd monitor in 1/10 of inches (50= 5")
// Can be overridden by command line -lcdsize=45
unsigned short LcdSize=50;

// The DpiSize, when 0 it is calculated by GetScreenDensity, otherwise forced 
// Can be overridden by command line -dpi=nnn 
unsigned short DpiSize=0; 
unsigned short ReferenceDpi=0;

// CommandQuantization is read by TerrainQuantization. Changing during runtime this
// parameter will permanently set quantization until it is changed again.
// It can also be forced by cmdline.
// Set this parameter to 0 to let TerrainQuantization() work normally.
// Note that TerrainQuantization() is called only when screen geometry changes.
unsigned int   CommandQuantization=0; 

// The time spent rendering the very first full map on startup
unsigned int DrawTerrainTimer=0; 

float ScreenPixelRatio=0;


int ScreenScale::screen_scale_int = ScreenScale::radix;
double ScreenScale::screen_scale_double = ScreenScale::radix;


unsigned short GetScreenGeometry(unsigned int x, unsigned int y);
double GetScreen0Ratio(void);
int GetScreenDensity(void);

// InitLKScreen can be called anytime, and should be called upon screen changed from portrait to landscape,
// or windows size is changed for any reason. We dont support dynamic resize of windows, though, because each
// resolution has its own tuned settings. This is thought for real devices, not for PC emulations.
// Attention: after InitLKScreen, also InitLKFonts should be called.

// NOTES
//
// ScreenScale (fixed point 22.10)
//
//   Ratio between the shortest size of the screen in pixels, and 240.
//   Range is 1 (240px) and up. We support any resolution starting from 240px.
//  
// IBLSCALE(x)
//  
//   Function for scaling inside screen, not dialogs.
//   Careful using 2*IBLSCALE(x) and not IBLSCALE(2*x)
//
// NIBLSCALE(x)
//
//   Deprecated, it's an alias of IBLSCALE(x). 
//
// DLGSCALE(x)
//   Same as IBLSCALE(x) but dedicated to dialog templates.
//   Inside dialogs it is MANDATORY to use DLGSCALE, for future compatibility in case
//   we change rescaling approach on map drawing. 
//
// ALL OF THE ABOVE is used to rescale properly in respect to the geometry of LK.
// We adopted for historical reasons 240 as base resolution (effectively the lowest, so it is the unity).
// But geometry (the size in pixels and the shape of the screen, 5:3, 16:9 etc.) is one thing, 
// size of screen is another matter. There are cases we want to enlarge things because the screen density
// is way too large in respect of the low resolutions existing in windows devices so far.
// So we now have also a ScreenPixelRatio (int) and relative RescalePixelSize() function for this purpose.
// ScreenPixelRatio is the difference in size of one pixel on current screen in respect of a 
// "standard" look chosen for a certain density. A 480x272 5" is around 110dpi, while a 800x480 5" is 186dpi.
// We assume a "reference dpi" is good for rescaling. 80, 96, 110, anything, and we use it.
//
// FONTS and Screen0Ratio
//
// We tuned geometries (4:3 5:3 16:9 etc) around templates and we rescale in their respect.
// If we need to rescale 1600x960 , ratio 1.66, we use 800x480 template for fonts and we rescale.
// Screen0Ratio is made for this purpose. And it is a vertical ratio, because we rescale fonts
// vertically (that's the way to do it). Screen0Ratio is otherwise pretty useless for the programmer.
// Since we manage each supported geometry separately, we also have a :
//
// ScreenGeometry
// a simple reference to the enumerated gemetries we manage. 
// If you need to rescale a bitmap, the original should be *optimistically* with the same 
// geometry of ScreenGeometry, otherwise you must stretch it. This is an important thing to know.
//
//


void InitLKScreen() {

#if (WINDOWSPC>0) || defined(__linux__)
    const PixelRect Rect(main_window->GetClientRect());
    ScreenSizeX = Rect.GetSize().cx;
    ScreenSizeY = Rect.GetSize().cy;
#else
    ScreenSizeX = GetSystemMetrics(SM_CXSCREEN);
    ScreenSizeY = GetSystemMetrics(SM_CYSCREEN);
#endif

    ScreenSize = ssnone; // This is "ssnone"

    // -----------------------------------------------------
    // These are the embedded known resolutions, fine tuned.
    // -----------------------------------------------------

    if (ScreenSizeX == 240 && ScreenSizeY == 320) ScreenSize = ss240x320; // QVGA      portrait
    if (ScreenSizeX == 234 && ScreenSizeY == 320) ScreenSize = ss240x320; // use the same config of 240x320
    if (ScreenSizeX == 272 && ScreenSizeY == 480) ScreenSize = ss272x480;
    if (ScreenSizeX == 240 && ScreenSizeY == 400) ScreenSize = ss240x320; //           portrait
    if (ScreenSizeX == 480 && ScreenSizeY == 640) ScreenSize = ss480x640; //  VGA
    if (ScreenSizeX == 640 && ScreenSizeY == 480) ScreenSize = ss640x480; //   VGA
    if (ScreenSizeX == 320 && ScreenSizeY == 240) ScreenSize = ss320x240; //  QVGA
    if (ScreenSizeX == 320 && ScreenSizeY == 234) ScreenSize = ss320x240; //  QVGA
    if (ScreenSizeX == 480 && ScreenSizeY == 800) ScreenSize = ss480x800;
    if (ScreenSizeX == 600 && ScreenSizeY == 800) ScreenSize = ss600x800;
    if (ScreenSizeX == 400 && ScreenSizeY == 240) ScreenSize = ss400x240; // landscape
    if (ScreenSizeX == 480 && ScreenSizeY == 272) ScreenSize = ss480x272; // WQVGA     landscape
    if (ScreenSizeX == 480 && ScreenSizeY == 234) ScreenSize = ss480x234; //   iGo
    if (ScreenSizeX == 800 && ScreenSizeY == 480) ScreenSize = ss800x480; //  WVGA
    if (ScreenSizeX == 800 && ScreenSizeY == 600) ScreenSize = ss800x600; //  WVGA


    ScreenGeometry = GetScreenGeometry(ScreenSizeX, ScreenSizeY);

    if (ScreenSize == ssnone) {
        StartupStore(_T(". InitLKScreen: AUTORES %dx%d%s"), ScreenSizeX, ScreenSizeY, NEWLINE);

        ScreenLandscape = (ScreenSizeX >= ScreenSizeY);

        // ScreenGeometry and ScreenLandscape need to be set before call GetScreen0Ratio;
        Screen0Ratio = GetScreen0Ratio();

    } else {
        StartupStore(_T(". InitLKScreen: %dx%d%s"), ScreenSizeX, ScreenSizeY, NEWLINE);

        ScreenLandscape = ((ScreenSize_t) ScreenSize > sslandscape);
        Screen0Ratio = 1;
    }

    
    // -----------------------------
    // Calculate Screen Scale Factor
    // -----------------------------
    int minsize = std::min(ScreenSizeX, ScreenSizeY);
    ScreenScale::set((double)minsize / 240.0);

    // This is used by RescalePixelSize(), defined in Makefile when needed.
    // Some functions using ScreenScale have been changed to use rescaled pixels.
    // We must check that pixelratio is never lower than ScreenScale.
    ScreenDensity = GetScreenDensity();
    if (!ReferenceDpi) ReferenceDpi=LK_REFERENCE_DPI;
    ScreenPixelRatio = (float)ScreenDensity/(float)ReferenceDpi;

    
    // -----------------------------
    // Initialize some Global variable 
    // -----------------------------
    
    
    // Initially, this is the default. Eventually retune it for each resolution.
    // We might in the future also set a UseStretch, with or without Hires.
    UseHiresBitmap = (IBLSCALE(1) > 1);

    //
    // The thinnest line somehow visible on screen from 35cm distance.
    //
    ScreenThinSize = IBLSCALE(1);

    GestureSize = IBLSCALE(50);

    // Override defaults for custom settings
    switch ((ScreenSize_t) ScreenSize) {
        case ss600x800:
            LKVarioSize = 45;
            break;
        case ss240x320:
            LKVarioSize = 13;
            break;
        case ss272x480:
            LKVarioSize = 30;
            break;
        default:
            if (ScreenLandscape) {
                LKVarioSize = ScreenSizeX / 16;
            } else {
                LKVarioSize = ScreenSizeX / 11;
            }
            break;
    }

    AircraftMenuSize = NIBLSCALE(28) + 14;
    CompassMenuSize = AircraftMenuSize + NIBLSCALE(17);

#ifdef TESTBENCH
    StartupStore(_T("..... ScreenSizeX      = %d" NEWLINE), ScreenSizeX);
    StartupStore(_T("..... ScreenSizeY      = %d" NEWLINE), ScreenSizeY);
    StartupStore(_T("..... ScreenDensity    = %d" NEWLINE), ScreenDensity);
    StartupStore(_T("..... ScreenGeometry   = %d" NEWLINE), ScreenGeometry);
    StartupStore(_T("..... ScreenSize(enum) = %d" NEWLINE), ScreenSize);
    StartupStore(_T("..... Screen0Ratio     = %f" NEWLINE), Screen0Ratio);
    StartupStore(_T("..... ScreenScale      = %f" NEWLINE), ScreenScale::get());

    StartupStore(_T("..... ReferenceDpi     = %d" NEWLINE), ReferenceDpi);
    StartupStore(_T("..... ScreenPixelRatio = %f" NEWLINE), ScreenPixelRatio);
    StartupStore(_T("..... UseHiresBitmap   = %s" NEWLINE), UseHiresBitmap?_T("true"):_T("false"));
    StartupStore(_T("..... ThinSize         = %d" NEWLINE), ScreenThinSize);
    StartupStore(_T("..... NIBLSCALE(1)     = %d" NEWLINE), NIBLSCALE(1));
    StartupStore(_T("..... NIBLSCALE(1.0)   = %f" NEWLINE), NIBLSCALE(1.0));
    StartupStore(_T("..... NIBLSCALE(2)     = %d" NEWLINE), NIBLSCALE(2));
    StartupStore(_T("..... NIBLSCALE(2.0)   = %f" NEWLINE), NIBLSCALE(2.0));
    // These are just reminders, since DLGSCALE is IBLSCALE
    StartupStore(_T("..... DLGSCALE(1)     = %d" NEWLINE), DLGSCALE(1));
    StartupStore(_T("..... DLGSCALE(2)     = %d" NEWLINE), DLGSCALE(2));

    StartupStore(_T("..... GestureSize      = %d" NEWLINE), GestureSize);
    StartupStore(_T("..... LKVarioSize      = %d" NEWLINE), LKVarioSize);
    StartupStore(_T("..... AircraftMenuSize = %d" NEWLINE), AircraftMenuSize);
    StartupStore(_T("..... CompassMenuSize  = %d" NEWLINE), CompassMenuSize);

    StartupStore(_T("..... TerrainQuantiz.  = %d" NEWLINE), TerrainQuantization());
#endif  
} // End of LKInitScreen


//
// Inside LKFonts we support special resolutions at best possible tuned settings.
// These resolutions are used as a base for resizing, considering their geometry ratio.
// Most modern screens have a 1,777 ratio, so in any case there is no need to think
// about dozens of geometries and we can take it easy with a simple approach here.
//

unsigned short GetScreenGeometry(unsigned int x, unsigned int y) {

#if TESTBENCH
    LKASSERT(x < 5000 && y < 5000);
#endif
    LKASSERT(x > 0 && y > 0);

    double ratio = x >= y ? x / (double) y : y / (double) x;

    //
    // Table of internally tuned ratios in LK8000
    //
    // Ratio   Aspect     Examples
    // -----   ------     --------
    // 1,333    4:3        320x240 640x480 800x600
    // 1,666    5:3        800x480
    // 1,777    16:9       480x272 960x540 1280x720 1920x1080
    // 2,05     2:1        480x234
    //
    // Aspect change thresholds:
    //
    // 1,000
    //   1,166
    // 1,333
    //   1,500
    // 1,666
    //   1,721
    // 1,777
    //   1,888
    // 2,000
    //

    // Here we decide which is the closest ratio
    while (1) {
        if (ratio < 1.166) return SCREEN_GEOMETRY_21; // yet unsupported SCREEN_GEOMETRY_SQUARED!
        if (ratio < 1.500) return SCREEN_GEOMETRY_43; // 1,33
        if (ratio < 1.721) return SCREEN_GEOMETRY_53; // 1,66
        if (ratio < 1.888) return SCREEN_GEOMETRY_169; // 1,77
        if (ratio < 2.112) return SCREEN_GEOMETRY_21;
        ratio /= 2;
    }
}

//
// We calculate the correct scaling factor based on vertical extension.
// That is because all fonts are rescaled by their height by the function
// ApplyFontSize() using formula:  new_height=(old_height * Screen0Ratio)
// If we change this function, let's update also ScreenGeometry.h for memo.
//

double GetScreen0Ratio(void) {
    double ratio = 0;
    if (ScreenLandscape) {
        switch (ScreenGeometry) {
            case SCREEN_GEOMETRY_43:
                ratio = (double) ScreenSizeY / 480.0;
                break;
            case SCREEN_GEOMETRY_53:
                ratio = (double) ScreenSizeY / 480.0;
                break;
            case SCREEN_GEOMETRY_169:
                ratio = (double) ScreenSizeY / 272.0;
                break;
            case SCREEN_GEOMETRY_21:
                ratio = (double) ScreenSizeY / 234.0;
                break;
            default:
                ratio = (double) ScreenSizeY / 272.0;
                break;
        }
    } else {
        switch (ScreenGeometry) {
            case SCREEN_GEOMETRY_43:
                ratio = (double) ScreenSizeY / 640.0;
                break;
            case SCREEN_GEOMETRY_53:
                ratio = (double) ScreenSizeY / 800.0;
                break;
            case SCREEN_GEOMETRY_169:
                ratio = (double) ScreenSizeY / 480.0;
                break;
            case SCREEN_GEOMETRY_21:
                ratio = (double) ScreenSizeY / 480.0;
                break;
            default:
                ratio = (double) ScreenSizeY / 480.0;
                break;
        }
    }
    return ratio;
}

//
// Screen DPI estimation for some platform.
//
int GetScreenDensity(void) {

    if (DpiSize) return((int)DpiSize);

#ifdef KOBO
    switch (DetectKoboModel()) {
        case KoboModel::GLOHD:
        case KoboModel::CLARAHD:
            return 300;
        case KoboModel::TOUCH2:
            return 167;
        case KoboModel::AURA2:
        case KoboModel::GLO:
            return 212;
        case KoboModel::AURA:
            return 213;
        case KoboModel::TOUCH:
            return 170;
        default:
            return 200; // Kobo Mini 200 dpi; Kobo Glo 212 dpi (according to Wikipedia)
    }
#endif

#ifdef  ANDROID
    return native_view->GetXDPI();
#endif

    //#ifdef WIN32    // for the moment we mantain default value for WIN32 as LOGPIXELSX always return 96 ?
    //	HDC dc = GetDC(NULL);
    //	return GetDeviceCaps(dc, LOGPIXELSX);
    //#endif

#if 0 // def USE_X11
    _XDisplay* disp = XOpenDisplay(NULL);
    double xres = ((((double) DisplayWidth(disp, 0)) * 25.4) / ((double) DisplayWidthMM(disp, 0)));
    XCloseDisplay(disp);
    return (short) xres;
#endif

    // if we are not able to get correct value just return default estimation
    return (sqrt(ScreenSizeX * ScreenSizeX + ScreenSizeY * ScreenSizeY) *10) / LcdSize; 
}


//
// This function is called by TerrainDraw TerrainRenderer.
// It happens every time we change screen resolution, like when we change multimaps.
// If we want a good result, either fix it manually like here or calculate dtquant dynamically.
// Since we have no dynamic calculation in v6.1, we go manual.
// TODO > use DrawTerrainTimer to choose optimal dtquant.
//
unsigned int TerrainQuantization(void) {

  if (CommandQuantization) return CommandQuantization;

  unsigned int dtquant;


  // 
  // KOBO
  // for faster terrain redraw, dtquant need to be 2 or 4 ...
  //
  #ifdef KOBO
  KoboModel model = DetectKoboModel();
  switch(model) {
     case KoboModel::GLO:
     case KoboModel::GLOHD:
        dtquant = 4;
        break;
     case KoboModel::MINI:
     case KoboModel::TOUCH:
     case KoboModel::TOUCH2:
        dtquant = 2;
        break;
     case KoboModel::UNKNOWN:
     default:
        dtquant = std::max<unsigned>(2, IBLSCALE(2));
        if (dtquant > 4) dtquant = 4; // .. but not too much
        break;
  };
  goto _ret;
  #endif

  //
  // WINDOWS PC 
  // 
  #if (WINDOWSPC>0)
  if (ScreenSizeX >1900) // beyond FULLHD
     dtquant=3;
  else
     dtquant=2;

  goto _ret;
  #endif

  // GENERAL PURPOSE APPROACH
  dtquant = Clamp(IBLSCALE(2U), 2U, 3U);

  // .. And the values above are the used as defaults. Then we must dynamically check
  // if we can adjust to better values by checking drawing times.
  // We can do it for 6.1g 

#if (WINDOWSPC>0) || (KOBO)
_ret: 
#endif
  // do something for all platforms here

  return dtquant;
}


