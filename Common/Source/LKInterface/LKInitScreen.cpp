/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "DoInits.h"
#include "ScreenGeometry.h"
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

// InitLKScreen can be called anytime, and should be called upon screen changed from portrait to landscape,
// or windows size is changed for any reason. We dont support dynamic resize of windows, though, because each
// resolution has its own tuned settings. This is thought for real devices, not for PC emulations.
// Attention: after InitLKScreen, also InitLKFonts should be called.
void InitLKScreen() {

  const PixelRect Rect(MainWindow.GetClientRect());
  int iWidth=0, iHeight=0;

#if (WINDOWSPC>0) || defined(__linux__)
  iWidth=Rect.GetSize().cx;
  iHeight=Rect.GetSize().cy;
#else
  iWidth=GetSystemMetrics(SM_CXSCREEN);
  iHeight=GetSystemMetrics(SM_CYSCREEN);
#endif

  ScreenSizeX=iWidth;
  ScreenSizeY=iHeight;
  ScreenSizeR.top=0;
  ScreenSizeR.bottom=iHeight-1;
  ScreenSizeR.left=0;
  ScreenSizeR.right=iWidth-1;


  int maxsize=0;
  int minsize=0;
  maxsize = max(ScreenSizeR.right-ScreenSizeR.left+1,ScreenSizeR.bottom-ScreenSizeR.top+1);
  minsize = min(ScreenSizeR.right-ScreenSizeR.left+1,ScreenSizeR.bottom-ScreenSizeR.top+1);

  ScreenDScale = max(1.0,minsize/240.0); // always start w/ shortest dimension

  if (maxsize == minsize)
  {
    ScreenDScale *= 240.0 / 320.0;
  }

  ScreenScale = (int)ScreenDScale;

  #if (WINDOWSPC>0)
  if (maxsize==720) {
        ScreenScale=2; // force rescaling with Stretch
  }
  #endif

  if ( ((double)ScreenScale) == ScreenDScale)
	ScreenIntScale = true;
  else
	ScreenIntScale = false;

  int i;
  if ( ScreenIntScale ) {
        for (i=0; i<=MAXIBLSCALE; i++) LKIBLSCALE[i]=(int)(i*ScreenScale);
  } else {
        for (i=0; i<=MAXIBLSCALE;i++) LKIBLSCALE[i]=(int)(i*ScreenDScale);
  }

  // Initially, this is the default. Eventually retune it for each resolution.
  // We might in the future also set a UseStretch, with or without Hires.
  if (ScreenScale>1)
	UseHiresBitmap=true;
  else
	UseHiresBitmap=false;

  #ifdef TESTBENCH
  StartupStore(_T("...... ScreenScale=%d ScreenDScale=%.3f ScreenIntScale=%d\n"),ScreenScale,ScreenDScale,ScreenIntScale);
  #endif

  ScreenSize=0; // This is "ssnone"

  // -----------------------------------------------------
  // These are the embedded known resolutions, fine tuned.
  // -----------------------------------------------------

  if (iWidth == 240 && iHeight == 320) ScreenSize=(ScreenSize_t)ss240x320; // QVGA      portrait
  if (iWidth == 234 && iHeight == 320) ScreenSize=(ScreenSize_t)ss240x320; // use the same config of 240x320
  if (iWidth == 272 && iHeight == 480) ScreenSize=(ScreenSize_t)ss272x480;
  if (iWidth == 240 && iHeight == 400) ScreenSize=(ScreenSize_t)ss240x320; //           portrait
  if (iWidth == 480 && iHeight == 640) ScreenSize=(ScreenSize_t)ss480x640; //  VGA
  if (iWidth == 640 && iHeight == 480) ScreenSize=(ScreenSize_t)ss640x480; //   VGA
  if (iWidth == 320 && iHeight == 240) ScreenSize=(ScreenSize_t)ss320x240; //  QVGA
  if (iWidth == 320 && iHeight == 234) ScreenSize=(ScreenSize_t)ss320x240; //  QVGA
  if (iWidth == 480 && iHeight == 800) ScreenSize=(ScreenSize_t)ss480x800;
  if (iWidth == 600 && iHeight == 800) ScreenSize=(ScreenSize_t)ss600x800;
  if (iWidth == 400 && iHeight == 240) ScreenSize=(ScreenSize_t)ss400x240; // landscape
  if (iWidth == 480 && iHeight == 272) ScreenSize=(ScreenSize_t)ss480x272; // WQVGA     landscape
  if (iWidth == 480 && iHeight == 234) ScreenSize=(ScreenSize_t)ss480x234; //   iGo
  if (iWidth == 800 && iHeight == 480) ScreenSize=(ScreenSize_t)ss800x480; //  WVGA
  if (iWidth == 800 && iHeight == 600) ScreenSize=(ScreenSize_t)ss800x600; //  WVGA

  ScreenGeometry=GetScreenGeometry(iWidth,iHeight);
  #if TESTBENCH
  StartupStore(_T("..... ScreenGeometry type: %d%s"),ScreenGeometry,NEWLINE);
  #endif

  if (ScreenSize==0) {
    StartupStore(_T(". InitLKScreen: AUTORES %dx%d%s"),iWidth,iHeight,NEWLINE);

	if (ScreenSizeX>=ScreenSizeY) {
		ScreenLandscape=true;
	} else {
		ScreenLandscape=false;
	}
        Screen0Ratio=GetScreen0Ratio();
	#if TESTBENCH
	StartupStore(_T("..... Screen0Ratio=%f\n"),Screen0Ratio);
	#endif
  } else {
    StartupStore(_T(". InitLKScreen: %dx%d%s"),iWidth,iHeight,NEWLINE);

    if (ScreenSize > (ScreenSize_t)sslandscape)
		ScreenLandscape=true;
	else
		ScreenLandscape=false;
  }
  ScreenDensity=GetScreenDensity();

  // This is used by RescalePixelSize(), defined in Makefile when needed.
  // Some functions using ScreenScale have been changed to use rescaled pixels.
  // We must check that pixelratio is never lower than ScreenScale.
  #ifdef RESCALE_PIXEL
  ScreenPixelRatio=(ScreenDensity*10)/LK_REFERENCE_DPI;
  #else
  ScreenPixelRatio=10;
  #endif
  int idscale=(int)ScreenDScale*10;
  ScreenPixelRatio=max<int>(idscale,ScreenPixelRatio);


  if (ScreenPixelRatio<10) {
     #ifdef TESTBENCH
     StartupStore(_T("... UNSUPPORTED RESCALING TO LOWER%s"),NEWLINE);
     #endif
     ScreenPixelRatio=10;
  }

  #ifdef TESTBENCH
  StartupStore(_T("... ScreenDensity= %d  idscale=%d ScreenPixelRatio=%d (/10) ThinSize=%d NIBLSCALE(1)=%d (2)=%d %s"),ScreenDensity,idscale,ScreenPixelRatio,ScreenThinSize,NIBLSCALE(1), NIBLSCALE(2),NEWLINE);
  #endif
  
  //
  // The thinnest line somehow visible on screen from 35cm distance.
  //
  ScreenThinSize=RescalePixelSize(1);

  GestureSize=RescalePixelSize(50);

  if (ScreenLandscape) {
	LKVarioSize=ScreenSizeX/16;
  } else {
	LKVarioSize=ScreenSizeX/11;
  }

  // Override defaults for custom settings
  switch (ScreenSize) {
	case (ScreenSize_t)ss600x800:
		LKVarioSize=45;
		break;
	case (ScreenSize_t)ss240x320:
		LKVarioSize=13;
		break;
	case (ScreenSize_t)ss272x480:
		LKVarioSize=30;
		break;
	default:
		break;
  }

  AircraftMenuSize=NIBLSCALE(28)+14;
  CompassMenuSize=AircraftMenuSize+NIBLSCALE(17);

} // End of LKInitScreen


//
// Inside LKFonts we support special resolutions at best possible tuned settings.
// These resolutions are used as a base for resizing, considering their geometry ratio.
// Most modern screens have a 1,777 ratio, so in any case there is no need to think
// about dozens of geometries and we can take it easy with a simple approach here.
//
unsigned short GetScreenGeometry(unsigned int x, unsigned int y) {

  #if TESTBENCH
  LKASSERT(x<5000 && y<5000);
  #endif
  LKASSERT(x>0 && y>0);

  double ratio= x>=y?x/(double)y:y/(double)x;

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
  while(1) {
      if ( ratio< 1.166) return SCREEN_GEOMETRY_21; // yet unsupported SCREEN_GEOMETRY_SQUARED!
      if ( ratio< 1.500) return SCREEN_GEOMETRY_43; // 1,33
      if ( ratio< 1.721) return SCREEN_GEOMETRY_53; // 1,66
      if ( ratio< 1.888) return SCREEN_GEOMETRY_169;// 1,77
      if ( ratio< 2.112) return SCREEN_GEOMETRY_21;
      ratio/=2;
  }
}

//
// We calculate the correct scaling factor based on vertical extension.
// That is because all fonts are rescaled by their height by the function
// ApplyFontSize() using formula:  new_height=(old_height * Screen0Ratio)
// If we change this function, let's update also ScreenGeometry.h for memo.
//
double GetScreen0Ratio(void) {
  double ratio=0;
  if (ScreenLandscape) {
      switch (ScreenGeometry) {
          case SCREEN_GEOMETRY_43:
              ratio=(double)ScreenSizeY/480.0;
              break;
          case SCREEN_GEOMETRY_53:
              ratio=(double)ScreenSizeY/480.0;
              break;
          case SCREEN_GEOMETRY_169:
              ratio=(double)ScreenSizeY/272.0;
              break;
          case SCREEN_GEOMETRY_21:
              ratio=(double)ScreenSizeY/234.0;
              break;
          default:
              ratio=(double)ScreenSizeY/272.0;
              break;
      }
  } else {
      switch (ScreenGeometry) {
          case SCREEN_GEOMETRY_43:
              ratio=(double)ScreenSizeY/640.0;
              break;
          case SCREEN_GEOMETRY_53:
              ratio=(double)ScreenSizeY/800.0;
              break;
          case SCREEN_GEOMETRY_169:
              ratio=(double)ScreenSizeY/480.0;
              break;
          case SCREEN_GEOMETRY_21:
              ratio=(double)ScreenSizeY/480.0;
              break;
          default:
              ratio=(double)ScreenSizeY/480.0;
              break;
      }
  }
  return ratio;
}

//
// Screen DPI estimation for some platform.
//
unsigned short GetScreenDensity(void) {

#ifdef KOBO
	switch (DetectKoboModel()) {
	case KoboModel::GLOHD:
		return 300;
	case KoboModel::TOUCH2:
		return 167;
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
	double xres = ((((double) DisplayWidth(disp,0)) * 25.4) /  ((double) DisplayWidthMM(disp,0)));
        XCloseDisplay(disp);
	return (short) xres;
#endif

	// if we are not able to get correct value just return default estimation
	return sqrt(ScreenSizeX*ScreenSizeX + ScreenSizeY*ScreenSizeY)/5; // default to a 5 in screen;
}



