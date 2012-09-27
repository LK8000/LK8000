/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "DoInits.h"


// InitLKScreen can be called anytime, and should be called upon screen changed from portrait to landscape,
// or windows size is changed for any reason. We dont support dynamic resize of windows, though, because each
// resolution has its own tuned settings. This is thought for real devices, not for PC emulations.
// Attention: after InitLKScreen, also InitLKFonts should be called. 
void InitLKScreen() {

  int iWidth=0, iHeight=0;

  #if (WINDOWSPC>0)
  iWidth=SCREENWIDTH;
  iHeight=SCREENHEIGHT;
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

  // StartupStore(_T("...... ScreenScale=%d ScreenDScale=%.3f ScreenIntScale=%d\n"),ScreenScale,ScreenDScale,ScreenIntScale);

  ScreenSize=0;

  if (iWidth == 240 && iHeight == 320) ScreenSize=(ScreenSize_t)ss240x320; // QVGA      portrait
  if (iWidth == 234 && iHeight == 320) ScreenSize=(ScreenSize_t)ss240x320; // use the same config of 240x320
  if (iWidth == 272 && iHeight == 480) ScreenSize=(ScreenSize_t)ss272x480;
  if (iWidth == 240 && iHeight == 400) ScreenSize=(ScreenSize_t)ss240x320; //           portrait

  if (iWidth == 480 && iHeight == 640) ScreenSize=(ScreenSize_t)ss480x640; //  VGA
  if (iWidth == 640 && iHeight == 480) ScreenSize=(ScreenSize_t)ss640x480; //   VGA
  if (iWidth == 320 && iHeight == 240) ScreenSize=(ScreenSize_t)ss320x240; //  QVGA
  if (iWidth == 320 && iHeight == 234) ScreenSize=(ScreenSize_t)ss320x240; //  QVGA
  if (iWidth == 720 && iHeight == 408) ScreenSize=(ScreenSize_t)ss720x408;
  if (iWidth == 480 && iHeight == 800) ScreenSize=(ScreenSize_t)ss480x800;
  if (iWidth == 400 && iHeight == 240) ScreenSize=(ScreenSize_t)ss400x240; // landscape
  if (iWidth == 480 && iHeight == 272) ScreenSize=(ScreenSize_t)ss480x272; // WQVGA     landscape
  if (iWidth == 480 && iHeight == 234) ScreenSize=(ScreenSize_t)ss480x234; //   iGo
  if (iWidth == 800 && iHeight == 480) ScreenSize=(ScreenSize_t)ss800x480; //  WVGA
  if (iWidth == 896 && iHeight == 672) ScreenSize=(ScreenSize_t)ss896x672; //  PC version only
  if (iWidth == 854 && iHeight == 358) ScreenSize=(ScreenSize_t)ss480x272; // use the same config 

  TCHAR tbuf[80];
  if (ScreenSize==0) {
        wsprintf(tbuf,_T(". InitLKScreen: ++++++ ERROR UNKNOWN RESOLUTION %dx%d !%s"),iWidth,iHeight,NEWLINE); // 091119
        StartupStore(tbuf);
  } else {
        wsprintf(tbuf,_T(". InitLKScreen: %dx%d%s"),iWidth,iHeight,NEWLINE); // 091213
        StartupStore(tbuf);
  }

  if (ScreenSize > (ScreenSize_t)sslandscape) 
	ScreenLandscape=true;
  else
	ScreenLandscape=false;

  // By default, h=v=size/6 and here we set it better
  switch (ScreenSize) { 
	case (ScreenSize_t)ss800x480:
		GestureSize=50;
		LKVarioSize=50;
		// dscale=480/240=2  800/dscale=400 -(70+2+2)=  326 x dscale = 652
		LKwdlgConfig=652;
		BottomSize=80; // Title+Value-4
		break;
	case (ScreenSize_t)ss400x240:
		GestureSize=50;
		LKVarioSize=25;
		// dscale=240/240=1  400/dscale=400 -(70+2+2)=  326 x dscale = 326
		LKwdlgConfig=326;
		BottomSize=40; // Title+Value-4
		break;
	case (ScreenSize_t)ss640x480:
		GestureSize=50;
		LKVarioSize=40;
		// dscale=480/240=2  640/dscale=320 -(70+2+2)=  246 x dscale = 492
		LKwdlgConfig=492;
		BottomSize=72; // Title+Value-4
		break;
	case (ScreenSize_t)ss896x672:
		GestureSize=50;
		LKVarioSize=56;
		// dscale=672/240=2.8  896/dscale=320 -(70+2+2)=  246 x dscale = 689
		LKwdlgConfig=689;
		BottomSize=78; // Title+Value-4
		break;
	case (ScreenSize_t)ss480x272:
		GestureSize=50;
		LKVarioSize=30;
		// dscale=272/240=1.133  480/dscale=424 -(70+2+2)=  350 x dscale = 397
		LKwdlgConfig=395;
		BottomSize=48; // Title+Value-4 plus something more
		break;
	case (ScreenSize_t)ss720x408:
		GestureSize=50;
		LKVarioSize=45;
		// dscale=408/240=1.133  720/dscale=423 -(70+2+2)=  350 x dscale = 594
		LKwdlgConfig=594;
		BottomSize=72; // Title+Value-4 plus something more
		break;
	case (ScreenSize_t)ss480x234:
		GestureSize=50;
		LKVarioSize=30;
		// dscale=234/240=0.975  480/dscale=492 -(70+2+2)=  418 x dscale = 407
		LKwdlgConfig=405;
		BottomSize=41; // Title+Value-4
		break;
	case (ScreenSize_t)ss320x240:
		GestureSize=50;
		LKVarioSize=20;
		// dscale=240/240=1  320/dscale=320 -(70+2+2)=  246 x dscale = 246
		// but 246 is too long..
		LKwdlgConfig=244;
		BottomSize=38;
		break;
	// PORTRAIT MODES
	case (ScreenSize_t)ss480x640:
		GestureSize=50;
		LKVarioSize=30;
		BottomSize=135;
		break;
	case (ScreenSize_t)ss480x800:
		GestureSize=50;
		LKVarioSize=30;
		// dscale=240/240=1  400/dscale=400 -(70+2+2)=  326 x dscale = 326
		LKwdlgConfig=324;
		BottomSize=135;
		break;
	case (ScreenSize_t)ss240x320:
		GestureSize=50;
		LKVarioSize=13;
		BottomSize=68;
		break;
	case (ScreenSize_t)ss272x480:
		GestureSize=50;
		LKVarioSize=30;
		BottomSize=80; // Title+Value-4  a bit bigger here
		break;
	default:
		GestureSize=50;
		LKVarioSize=30;
		BottomSize=38; // Title+Value-4
		break;
  }

  // Used by MapWndProc touch key ckecks to know if a key is on the bottombar
  // We can use Y_BottomBar here
  BottomBarY=ScreenSizeY-BottomSize-NIBLSCALE(2);

  AircraftMenuSize=NIBLSCALE(28)+14;
  CompassMenuSize=AircraftMenuSize+NIBLSCALE(17);

} // End of LKInitScreen


