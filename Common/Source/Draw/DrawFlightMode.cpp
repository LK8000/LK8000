/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "DoInits.h"
#include "Bitmaps.h"
#include "Multimap.h"
#include "Asset.hpp"


extern bool FastZoom; // QUICKDRAW

#define UTF_THERMALING 11
#define UTF_CRUISE     12 
#define UTF_FINALGLIDE 13
static constexpr unsigned mmUTF8Symbol[] = {
    2350, // _@M2350_ "Ⓜ"
    2351, // _@M2351_ "①"
    2352, // _@M2352_ "②"
    2353, // _@M2353_ "③"
    2354, // _@M2354_ "④"
    2355, // _@M2355_ "⑤"
    2356, // _@M2356_ "⑥"
    2357, // _@M2357_ "⑦"
    2358, // _@M2358_ "⑧"
    2359, // _@M2359_ "⑨"
    2360, // _@M2360_ "⑩"
    2376, // _@M2376_ "☁️"    // THERMALING
    2384, // _@M2384_ "✈"    // CRUISE
    2377, // _@M2377_ "⚑"    // FINALGLIDE
    
};

static constexpr LKColor mmUTF8Color[] = {
    LKColor(0,245,255),  // "Ⓜ"
    LKColor(0,255,127),  // "①"
    LKColor(205,0,0),    // "②"
    LKColor(255,130,71), // "③"
    LKColor(255,195,0),  // "④"
    LKColor(255,30,171), // "⑤"
    LKColor(82,82,82),   // "⑥"
    RGB_DARKBLUE,        // "⑦"
    RGB_BLACK,           // "⑧"
    RGB_BLACK,           // "⑨"
    RGB_BLACK,           // "⑩"
    RGB_LIGHTBLUE,       // "☁️"    // THERMALING
    RGB_YELLOW,          // "✈"    // CROUISE
    RGB_GREEN,           // "⚑"    // FINALGLIDE
};

static_assert(std::size(mmUTF8Color) == std::size(mmUTF8Symbol), "invalide array size");

static std::pair<const LKColor, const TCHAR*> GetUTF8MultimapSymbol(unsigned Number) {
    assert(Number < std::size(mmUTF8Symbol));

    const unsigned symbol_idx = Number < std::size(mmUTF8Symbol) ? Number : 0;
    const TCHAR* symbol = MsgToken(mmUTF8Symbol[symbol_idx]);

    LKColor color = mmUTF8Color[symbol_idx];
    if (IsDithered()) {
      color = INVERTCOLORS ? RGB_WHITE : RGB_BLACK ;
    }

    return std::make_pair(color, symbol);
}



void UTF8DrawMultimapSymbol(LKSurface& Surface, const RasterPoint& pos, const PixelSize& size, int Number)
{  
    const auto Pict = GetUTF8MultimapSymbol(Number);
    const auto OldFont =  Surface.SelectObject(LK8PanelBigFont);

    LKColor Color  = Pict.first;
    PixelRect textRect(pos.x - 0.2f*size.cx , pos.y- 0.2f*size.cy, pos.x + size.cx,  pos.y +  size.cy);
    
    if(!INVERTCOLORS) Color = Color.MixColors( RGB_BLACK,  0.6f );  // make a little darker for white background
    
    const auto OldColor = Surface.SetTextColor(Color);
 
    Surface.DrawText(Pict.second, &textRect, DT_CENTER|DT_VCENTER );

    Surface.SelectObject(OldFont);
    Surface.SetTextColor(OldColor);
}


// locals
static unsigned short use_rescale=0;
static double newscale=1;

static int rescale(int n) {
  switch (use_rescale) {
      case 0:
          return n;
          break;
      case 1:
          return IBLSCALE(n);
          break;
      case 2:
          return (n*newscale);
          break;
      default:
          break;
  }
  LKASSERT(0);
  return n; // impossible
}

//
// Bottom right corner indicators: Flight mode, Battery, Logger
//
void MapWindow::DrawFlightMode(LKSurface& Surface, const RECT& rc)
{
  static bool flip= true;
  static PixelSize loggerIconSize,mmIconSize, batteryIconSize;
  static RasterPoint loggerPoint, mmPoint, batteryPoint;
  static PixelSize loggerNewSize, mmNewSize, batteryNewSize;
  static int vsepar;

  LKIcon* ptmpBitmap = NULL;

  if (DoInit[MDI_DRAWFLIGHTMODE]) {
      DoInit[MDI_DRAWFLIGHTMODE]=false;

      ptmpBitmap=&hLogger;
      loggerIconSize = ptmpBitmap->GetSize();

      ptmpBitmap=&hMM0;;
      mmIconSize = ptmpBitmap->GetSize();

      ptmpBitmap=&hBattery12;;
      batteryIconSize = ptmpBitmap->GetSize();

      //
      // determine if we can rescale. Preference is to keep standard rescale.
      //
      vsepar=NIBLSCALE(1);
      #define HSEPAR NIBLSCALE(1)  // right border and items separator

      use_rescale=1; // IBLSCALING
      int minvsize= vsepar + rescale(mmIconSize.cy) + rescale(batteryIconSize.cy)+vsepar;
      if (minvsize > BottomSize) {
          use_rescale=0; // NO SCALING
          minvsize= vsepar + rescale(mmIconSize.cy) + rescale(batteryIconSize.cy)+vsepar;
          if (minvsize >= BottomSize) {
              vsepar=0; // minimize interlines
              minvsize= rescale(mmIconSize.cy) + rescale(batteryIconSize.cy);
              LKASSERT(minvsize>0);
              if (minvsize<=1) minvsize=BottomSize; // recover impossible error
              newscale=BottomSize/(double)minvsize;
              use_rescale=2;
          } else {
              // using unscaled bitmaps the BottomSize is taller than minvsize;
              // lets see if we can enlarge them a bit. We cannot exceed the BB_ICONSIZE
              newscale=BottomSize/(double)minvsize;
              use_rescale=2;
              int minhsize= rescale(batteryIconSize.cx) + HSEPAR + rescale(loggerIconSize.cx) + HSEPAR;
              if (minhsize > (NIBLSCALE(26)+1)) { // BB_ICONSIZE! with tolerance 1
                  for (; newscale>1; newscale-=0.1) {
                      minhsize= rescale(batteryIconSize.cx) + HSEPAR + rescale(loggerIconSize.cx) + HSEPAR;
                      if (minhsize<= (NIBLSCALE(26)+1)) break;
                  }
                  if (newscale <= 1) use_rescale=0; // give up, keep small bitmaps
              }
          }

      }


      //
      // precalculate positions and sizes
      //

      loggerPoint.x=rc.right-rescale(loggerIconSize.cx)-HSEPAR;
      // center the logger icon in respect of battery icon which is bigger
      loggerPoint.y=rc.bottom - BottomSize + rescale((batteryIconSize.cy-loggerIconSize.cy)/2) + vsepar;
      loggerNewSize.cx= rescale(loggerIconSize.cx);
      loggerNewSize.cy= rescale(loggerIconSize.cy);

      batteryPoint.x= loggerPoint.x - rescale(batteryIconSize.cx) - HSEPAR;
      batteryPoint.y= rc.bottom - BottomSize + vsepar;
      batteryNewSize.cx= rescale(batteryIconSize.cx);
      batteryNewSize.cy= rescale(batteryIconSize.cy);

      mmPoint.x= rc.right - rescale(mmIconSize.cx) - HSEPAR;
      mmPoint.x-=   (mmPoint.x - batteryPoint.x)/2;
      mmPoint.y= rc.bottom - rescale(mmIconSize.cy)- vsepar;
      mmNewSize.cx= rescale(mmIconSize.cx);
      mmNewSize.cy= rescale(mmIconSize.cy);

      // fine tuning for vertical spacing between items
      int interline=  mmPoint.y - (batteryPoint.y+rescale(batteryIconSize.cy));
      if (interline>4) {
          loggerPoint.y += (interline/4);
          batteryPoint.y += (interline/4);
          mmPoint.y -= (interline/4);
      }

  } // endof doinit


  //
  // Logger indicator
  //
  flip = !flip;

  if (DisableAutoLogger) {
      ptmpBitmap = &hLoggerDisabled;
  } else {
      if (LoggerActive) {
          ptmpBitmap = &hLogger;
      } else {
          if (flip)
              ptmpBitmap = &hLoggerOff;
          else
              ptmpBitmap = &hLoggerDisabled;
      }
  }
  if ( (!DisableAutoLogger && (LoggerActive || !flip)) && ptmpBitmap) {
      ptmpBitmap->Draw(Surface, loggerPoint.x, loggerPoint.y, loggerNewSize.cx, loggerNewSize.cy);
  }

  //
  // Big icon
  //
  if(Appearance.UTF8Pictorials == false) {

    if (!IsMultiMapNoMain() && mode.Is(Mode::MODE_CIRCLING)) {
   
        ptmpBitmap = &hClimb;
    } else {
        //short i=Get_Current_Multimap_Type()-1;
        short i=ModeType[LKMODE_MAP]-1;
        if (i<0) i=0;
        if (!IsMultiMap()) {
            switch(i) {
                case 0:
                    ptmpBitmap = &hIMM0;
                    break;
                case 1:
                    ptmpBitmap = &hIMM1;
                    break;
                case 2:
                    ptmpBitmap = &hIMM2;
                    break;
                case 3:
                    ptmpBitmap = &hIMM3;
                    break;
                case 4:
                    ptmpBitmap = &hIMM4;
                    break;
                case 5:
                    ptmpBitmap = &hIMM5;
                    break;
                case 6:
                    ptmpBitmap = &hIMM6;
                    break;
                case 7:
                    ptmpBitmap = &hIMM7;
                    break;
                case 8:
                    ptmpBitmap = &hIMM8;
                    break;
                default:
                    ptmpBitmap = &hIMM0;
                    break;
            }
        } else {
            switch(i) {
                case 0:
                    ptmpBitmap = &hMM0;
                    break;
                case 1:
                    ptmpBitmap = &hMM1;
                    break;
                case 2:
                    ptmpBitmap = &hMM2;
                    break;
                case 3:
                    ptmpBitmap = &hMM3;
                    break;
                case 4:
                    ptmpBitmap = &hMM4;
                    break;
                case 5:
                    ptmpBitmap = &hMM5;
                    break;
                case 6:
                    ptmpBitmap = &hMM6;
                    break;
                case 7:
                    ptmpBitmap = &hMM7;
                    break;
                case 8:
                    ptmpBitmap = &hMM8;
                    break;
                default:
                    ptmpBitmap = &hMM0;
                    break;
            }
        }
    }

    if(ptmpBitmap ) {
      ptmpBitmap->Draw(Surface, mmPoint.x, mmPoint.y, mmNewSize.cx,mmNewSize.cy);
    }
  }
  else
  {
   short i=ModeType[LKMODE_MAP]-1;
   //if (!IsMultiMapNoMain() && mode.Is(Mode::MODE_CRUISE)) i = UTF_CROUISE;
   if (!IsMultiMapNoMain() && mode.Is(Mode::MODE_CIRCLING)) i = UTF_THERMALING;
   if (!IsMultiMapNoMain() && mode.Is(Mode::MODE_FINAL_GLIDE)) i = UTF_FINALGLIDE;

   UTF8DrawMultimapSymbol( Surface, mmPoint, mmNewSize, i);
  }
  //
  // Battery indicator
  //

  #if TESTBENCH && !defined(KOBO)
  // Battery test in Simmode will be available in testbench mode only
  if (!HaveBatteryInfo && SIMMODE && !(QUICKDRAW)) {
      PDABatteryPercent-=1;
      if (PDABatteryPercent<0) {
          PDABatteryPercent=100;
      }
  }
  #else
  // If we are not in testbench, no matter simmode is active we shall represent the real battery (as in v5).
  // Exception: PC version.
  #if (WINDOWSPC>0)
  if (!SIMMODE) return;
  #else
  if (!HaveBatteryInfo) return;
  #endif
  #endif

  if ((PDABatteryPercent==0 || PDABatteryPercent>100) && PDABatteryStatus==Battery::ONLINE && PDABatteryFlag!=Battery::CHARGING) {
    if (IsDithered() && !INVERTCOLORS) {
        ptmpBitmap = &hKB_BatteryFullC; 
    } else {
	    ptmpBitmap = &hBatteryFullC;
    }
	goto _drawbattery;
  }

  if (PDABatteryPercent<=6) {
	if (flip) return;
    if (IsDithered() && !INVERTCOLORS) {
        ptmpBitmap = &hKB_Battery12; 
    } else {
	    ptmpBitmap = &hBattery12;
    }
	goto _drawbattery;
  }

  if (PDABatteryPercent<=12) {
    if (IsDithered() && !INVERTCOLORS) {
        ptmpBitmap = &hKB_Battery12; 
    } else {
	    ptmpBitmap = &hBattery12;
    }
	goto _drawbattery;
  }
  if (PDABatteryPercent<=24) {
    if (IsDithered() && !INVERTCOLORS) {
        ptmpBitmap = &hKB_Battery24; 
    } else {
	    ptmpBitmap = &hBattery24;
    }
	goto _drawbattery;
  }
  if (PDABatteryPercent<=36) {
    if (IsDithered() && !INVERTCOLORS) {
        ptmpBitmap = &hKB_Battery36;
    } else {
    	ptmpBitmap = &hBattery36;
    }
	goto _drawbattery;
  }
  if (PDABatteryPercent<=48) {
    if (IsDithered() && !INVERTCOLORS) {
        ptmpBitmap = &hKB_Battery48;
    } else {
    	ptmpBitmap = &hBattery48;
    }
	goto _drawbattery;
  }
  if (PDABatteryPercent<=60) {
    if (IsDithered() && !INVERTCOLORS) {
        ptmpBitmap = &hKB_Battery60;
    } else {
	    ptmpBitmap = &hBattery60;
    }
	goto _drawbattery;
  }
  if (PDABatteryPercent<=72) {
    if (IsDithered() && !INVERTCOLORS) {
        ptmpBitmap = &hKB_Battery72; 
    } else {
    	ptmpBitmap = &hBattery72;
    }
	goto _drawbattery;
  }
  if (PDABatteryPercent<=84) {
    if (IsDithered() && !INVERTCOLORS) {
        ptmpBitmap = &hKB_Battery84; 
    } else {
	    ptmpBitmap = &hBattery84;
    }
	goto _drawbattery;
  }
  if (PDABatteryPercent<=96) {
    if (IsDithered() && !INVERTCOLORS) {
        ptmpBitmap = &hKB_Battery96;
    } else {
    	ptmpBitmap = &hBattery96;
    }
	goto _drawbattery;
  }
  if (PDABatteryStatus==Battery::ONLINE) {
    if (IsDithered() && !INVERTCOLORS) {
        ptmpBitmap = &hKB_BatteryFullC; 
    } else {
	    ptmpBitmap = &hBatteryFullC;
    }
  } else {
    if (IsDithered() && !INVERTCOLORS) {
        ptmpBitmap = &hKB_BatteryFull; 
    } else {
	    ptmpBitmap = &hBatteryFull;
    }
  }

_drawbattery:
    if(ptmpBitmap) {
        ptmpBitmap->Draw(Surface, batteryPoint.x, batteryPoint.y, batteryNewSize.cx, batteryNewSize.cy);
    }
}
