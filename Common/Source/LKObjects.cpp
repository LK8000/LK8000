/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKObjects.cpp,v 1.1 2010/12/11 19:21:43 root Exp root $
*/

//  Create common shared graphic objects, from MapWindow

#include "externs.h"

#define STATIC_LKOBJECTS
#include "LKObjects.h"
#include "Bitmaps.h"
#include "ScreenGeometry.h"

#include "RGB.h"

#include "utils/stl_utils.h"
using std::placeholders::_1;

void LKObjects_Create() {

  #if TESTBENCH
  StartupStore(_T("... LKObjects_Create\n"));
  #endif

  // CUSTOM BRUSHES
  LKBrush_Petrol.Create(RGB_PETROL);
  #ifdef DITHER
  LKBrush_LightGreen.Create(RGB_WHITE);
  LKBrush_Petrol.Create(RGB_BLACK);
  #else
  LKBrush_LightGreen.Create(RGB_LIGHTGREEN);
  LKBrush_Petrol.Create(RGB_PETROL);
  #endif
  LKBrush_DarkGreen.Create(RGB_DARKGREEN);
  LKBrush_Ndark.Create(RGB_NDARK);
  #ifdef DITHER
  LKBrush_Nlight.Create(RGB_WHITE);
  #else
  LKBrush_Nlight.Create(RGB_NLIGHT);
  #endif
  LKBrush_Mdark.Create(RGB_MDARK);
  LKBrush_Mlight.Create(RGB_MLIGHT);
  LKBrush_Red.Create(RGB_RED);
  LKBrush_Yellow.Create(RGB_YELLOW);
  #ifdef DITHER
  LKBrush_LightYellow.Create(RGB_WHITE);
  #else
  LKBrush_LightYellow.Create(RGB_LIGHTYELLOW);
  #endif
  LKBrush_Green.Create(RGB_GREEN);
  LKBrush_DarkYellow2.Create(RGB_DARKYELLOW2);
  #ifdef DITHER
  LKBrush_Orange.Create(RGB_GREY);
  #else
  LKBrush_Orange.Create(RGB_ORANGE);
  #endif
  LKBrush_Lake.Create(RGB_LAKE);
  LKBrush_Blue.Create(RGB_BLUE);
  LKBrush_Indigo.Create(RGB_INDIGO);
  LKBrush_LightGrey.Create(RGB_LIGHTGREY);
  LKBrush_DarkGrey.Create(LKColor(100,100,100));
  LKBrush_LcdGreen.Create(RGB_LCDGREEN);
  LKBrush_LcdDarkGreen.Create(RGB_LCDDARKGREEN);
  LKBrush_Grey.Create(RGB_GREY);
  LKBrush_Emerald.Create(RGB_EMERALD);
  LKBrush_DarkSlate.Create(RGB_DARKSLATE);
  LKBrush_LightCyan.Create(RGB_LIGHTCYAN);
  LKBrush_RifleGrey.Create(RGB_RIFLEGREY);

  LKBrush_Vario_neg4.Create(RGB_BLUE.ChangeBrightness(0.4));
  LKBrush_Vario_neg3.Create(RGB_BLUE.ChangeBrightness(0.6));
  LKBrush_Vario_neg2.Create(RGB_BLUE.ChangeBrightness(0.8));
  LKBrush_Vario_neg1.Create(RGB_BLUE.ChangeBrightness(1.0));
  LKBrush_Vario_0   .Create(RGB_YELLOW.ChangeBrightness(0.8));
  LKBrush_Vario_pos1.Create(RGB_GREEN.ChangeBrightness(0.6));
  LKBrush_Vario_pos2.Create(RGB_GREEN.ChangeBrightness(0.7));
  LKBrush_Vario_pos3.Create(RGB_GREEN.ChangeBrightness(0.8));
  LKBrush_Vario_pos4.Create(RGB_GREEN.ChangeBrightness(1.0));
  
  // Contextual LKBrush
  LKBrush_Higlighted.Create(RGB_HIGHTLIGHT);
  LKBrush_FormBackGround.Create(RGB_WINBACKGROUND);
  
  // CUSTOM PENS
  LKPen_Black_N0.Create(PEN_SOLID,ScreenThinSize,RGB_BLACK);
  LKPen_Black_N1.Create(PEN_SOLID,NIBLSCALE(1),RGB_BLACK);
  LKPen_Black_N2.Create(PEN_SOLID,NIBLSCALE(2),RGB_BLACK);
  LKPen_Black_N3.Create(PEN_SOLID,NIBLSCALE(3),RGB_BLACK);
  LKPen_Black_N4.Create(PEN_SOLID,NIBLSCALE(4),RGB_BLACK);
  LKPen_Black_N5.Create(PEN_SOLID,NIBLSCALE(5),RGB_BLACK);

  LKPen_White_N0.Create(PEN_SOLID,ScreenThinSize,RGB_WHITE);
  LKPen_White_N1.Create(PEN_SOLID,NIBLSCALE(1),RGB_WHITE);
  LKPen_White_N2.Create(PEN_SOLID,NIBLSCALE(2),RGB_WHITE);
  LKPen_White_N3.Create(PEN_SOLID,NIBLSCALE(3),RGB_WHITE);
  LKPen_White_N4.Create(PEN_SOLID,NIBLSCALE(4),RGB_WHITE);
  LKPen_White_N5.Create(PEN_SOLID,NIBLSCALE(5),RGB_WHITE);

  LKPen_Petrol_C2.Create(PEN_SOLID,NIBLSCALE(1)+2,RGB_PETROL);

  LKPen_Green_N1.Create(PEN_SOLID,NIBLSCALE(1),RGB_GREEN);
  LKPen_Red_N1.Create(PEN_SOLID,NIBLSCALE(1),RGB_RED);
  LKPen_Blue_N1.Create(PEN_SOLID,NIBLSCALE(1),RGB_BLUE);

  LKPen_Grey_N0.Create(PEN_SOLID,ScreenThinSize,RGB_GREY);
  LKPen_Grey_N1.Create(PEN_SOLID,NIBLSCALE(1),RGB_GREY);
  LKPen_Grey_N2.Create(PEN_SOLID,NIBLSCALE(2),RGB_GREY);

  switch(ScreenSize) {
	// portrait small screen
	case ss240x320:
	case ss240x400:
	case ss272x480:
	// landscape small screen
	case ss320x240:
	case ss400x240:
	case ss480x272:
		MapWindow::hpAircraft = LKPen_Black_N5; // LK v4 has it bolder
		LKPen_GABRG.Create(PEN_SOLID,NIBLSCALE(5),RGB_MAGENTA);
		break;
	default:
		MapWindow::hpAircraft = LKPen_Black_N4; // up to LK version 3
		LKPen_GABRG.Create(PEN_SOLID,NIBLSCALE(3),RGB_MAGENTA);
		break;
  }

  //
  // MapWindow objects
  //

  for (unsigned i=0; i<AIRSPACECLASSCOUNT; i++) {
	LKASSERT( MapWindow::iAirspaceColour[i]< NUMAIRSPACECOLORS);

    const LKColor& Color = MapWindow::Colours[MapWindow::iAirspaceColour[i]];
    
	MapWindow::hAirspacePens[i].Create(PEN_SOLID, NIBLSCALE(1), Color);
	MapWindow::hBigAirspacePens[i].Create(PEN_SOLID, NIBLSCALE(3),Color.ChangeBrightness(0.75));
  }
  MapWindow::hAirspaceBorderPen.Create(PEN_SOLID, NIBLSCALE(10), RGB_WHITE);

  //
  // THE SNAIL TRAIL 
  //
  // Default sizes on 320x240 practically equivalent to 9,4,3 on V5 480x272 (5" lcd)
  // That's because the trail was tuned to be visible on a 3.5" screen at 320x240 (114dpi)
  // and 480x272 on a 5" is almost the same (110dpi)
  //
  #define SNAIL_SIZE0  8.00  // 8
  #define SNAIL_SIZE5  3.60  // 4
  #define SNAIL_SIZE7  2.65  // 3
  #define SNAIL_SIZEN  6.00  // fixed color snail for low zoom map

  unsigned int iSnailSizes[NUMSNAILCOLORS+1];
  LKColor      hSnailColours[NUMSNAILCOLORS+1];

  extern float ScreenPixelRatio;

  int tmpsize= iround(SNAIL_SIZE0 * ScreenPixelRatio);
  #ifdef TESTBENCH
  StartupStore(_T(". SNAIL[0]=%d ratio=%f\n"),tmpsize,ScreenPixelRatio);
  #endif
  iSnailSizes[0]= tmpsize;
  iSnailSizes[1]= tmpsize;
  iSnailSizes[2]= tmpsize;
  iSnailSizes[3]= tmpsize;
  iSnailSizes[4]= tmpsize;
  iSnailSizes[10]= tmpsize;
  iSnailSizes[11]= tmpsize;
  iSnailSizes[12]= tmpsize;
  iSnailSizes[13]= tmpsize;
  iSnailSizes[14]= tmpsize;

  tmpsize= iround(SNAIL_SIZE5 * ScreenPixelRatio);
  #ifdef TESTBENCH
  StartupStore(_T(". SNAIL[5]=%d\n"),tmpsize);
  #endif
  iSnailSizes[5]= tmpsize;
  iSnailSizes[6]= tmpsize;
  iSnailSizes[9]= tmpsize;
  iSnailSizes[8]= tmpsize;

  tmpsize= iround(SNAIL_SIZE7 * ScreenPixelRatio);
  #ifdef TESTBENCH
  StartupStore(_T(". SNAIL[7]=%d\n"),tmpsize);
  #endif
  iSnailSizes[7]= tmpsize;

  tmpsize= iround(SNAIL_SIZEN * ScreenPixelRatio);
  #ifdef TESTBENCH
  StartupStore(_T(". (N) SNAIL[15]=%d\n"),tmpsize);
  #endif
  iSnailSizes[15]= tmpsize;


#ifndef DITHER

  // COLORED SNAIL TRAIL
  //
  hSnailColours[0] = RGB_BLACK;
  hSnailColours[1] = RGB_INDIGO;
  hSnailColours[2] = RGB_INDIGO;
  hSnailColours[3] = RGB_BLUE;
  hSnailColours[4] = RGB_BLUE;
  hSnailColours[5] = RGB_LAKE;
  hSnailColours[6] = RGB_LAKE;
  hSnailColours[7] = RGB_GREY;
  hSnailColours[8] = RGB_GREEN;
  hSnailColours[9] = RGB_GREEN;
  hSnailColours[10] = RGB_ORANGE;
  hSnailColours[11] = RGB_ORANGE;
  hSnailColours[12] = RGB_RED;
  hSnailColours[13] = RGB_RED;
  hSnailColours[14] = RGB_DARKRED;

  hSnailColours[15] = RGB_PETROL; // low zoom fixed color

  for (int i=0; i<NUMSNAILCOLORS+1; i++) {
     MapWindow::hSnailPens[i].Create(PEN_SOLID, iSnailSizes[i], hSnailColours[i]);
  }


#else

  // DITHERED SNAIL TRAIL
  // 
  hSnailColours[0] = RGB_GREY;
  hSnailColours[1] = RGB_GREY;
  hSnailColours[2] = RGB_GREY;
  hSnailColours[3] = RGB_GREY;
  hSnailColours[4] = RGB_GREY;
  hSnailColours[5] = RGB_GREY;
  hSnailColours[6] = RGB_GREY;
  hSnailColours[7] = RGB_WHITE;
  hSnailColours[8] =  RGB_BLACK;
  hSnailColours[9] =  RGB_RED;
  hSnailColours[10] = RGB_RED;
  hSnailColours[11] = RGB_RED;
  hSnailColours[12] = RGB_BLACK;
  hSnailColours[13] = RGB_BLACK;
  hSnailColours[14] = RGB_BLACK;
  hSnailColours[15] = RGB_BLACK;


  MapWindow::hSnailPens[0].Create(PEN_DASH, iSnailSizes[0], hSnailColours[0]);
  MapWindow::hSnailPens[1].Create(PEN_DASH, iSnailSizes[1], hSnailColours[1]);
  MapWindow::hSnailPens[2].Create(PEN_DASH, iSnailSizes[2], hSnailColours[2]);
  MapWindow::hSnailPens[3].Create(PEN_DASH, iSnailSizes[3], hSnailColours[3]);
  MapWindow::hSnailPens[4].Create(PEN_DASH, iSnailSizes[4], hSnailColours[4]);
  MapWindow::hSnailPens[5].Create(PEN_DASH, iSnailSizes[5], hSnailColours[5]);
  MapWindow::hSnailPens[6].Create(PEN_DASH, iSnailSizes[6], hSnailColours[6]);

  MapWindow::hSnailPens[7].Create(PEN_SOLID,  iSnailSizes[7], hSnailColours[7]);
  MapWindow::hSnailPens[8].Create(PEN_SOLID,  iSnailSizes[8], hSnailColours[8]);
  MapWindow::hSnailPens[9].Create(PEN_SOLID,  iSnailSizes[9], hSnailColours[9]);
  MapWindow::hSnailPens[10].Create(PEN_SOLID,  iSnailSizes[10], hSnailColours[10]);
  MapWindow::hSnailPens[11].Create(PEN_SOLID,  iSnailSizes[11], hSnailColours[11]);
  MapWindow::hSnailPens[12].Create(PEN_SOLID,  iSnailSizes[12], hSnailColours[12]);
  MapWindow::hSnailPens[13].Create(PEN_SOLID,  iSnailSizes[13], hSnailColours[13]);
  MapWindow::hSnailPens[14].Create(PEN_SOLID,  iSnailSizes[14], hSnailColours[14]);
  MapWindow::hSnailPens[15].Create(PEN_SOLID,  iSnailSizes[15], hSnailColours[15]);


#endif  // DITHERED


  for (unsigned i=0; i<array_size(MapWindow::hAirspaceBrushes); ++i) {
#ifdef HAVE_HATCHED_BRUSH
      static_assert(array_size(MapWindow::hAirspaceBrushes) == array_size(hAirspaceBitmap), "Array Size error");
      MapWindow::hAirspaceBrushes[i].Create(hAirspaceBitmap[i]);
#else
      static_assert(array_size(MapWindow::hAirspaceBrushes) == array_size(MapWindow::Colours), "Array Size error");
      MapWindow::hAirspaceBrushes[i].Create(MapWindow::Colours[i].WithAlpha(0xFF/2));
#endif
  }

#ifdef ENABLE_OPENGL
  MapWindow::AboveTerrainColor = RGB_GREY.WithAlpha(0xFF/2);
#else
#ifdef HAVE_HATCHED_BRUSH
  MapWindow::hAboveTerrainBrush.Create(hAboveTerrainBitmap);
#else
  MapWindow::hAboveTerrainBrush.Create(RGB_GREY);
#endif
#endif
  
  if(LKSurface::AlphaBlendSupported()) {
      MapWindow::InitAirSpaceSldBrushes(MapWindow::Colours);
  }
  

  MapWindow::hInvBackgroundBrush[0] = LKBrush_White;
  MapWindow::hInvBackgroundBrush[1] = LKBrush_LightGrey;
  MapWindow::hInvBackgroundBrush[2] = LKBrush_LcdGreen;
  MapWindow::hInvBackgroundBrush[3] = LKBrush_LcdDarkGreen;
  MapWindow::hInvBackgroundBrush[4] = LKBrush_Grey;
  MapWindow::hInvBackgroundBrush[5] = LKBrush_Lake;
  MapWindow::hInvBackgroundBrush[6] = LKBrush_Emerald;
  MapWindow::hInvBackgroundBrush[7] = LKBrush_DarkSlate;
  MapWindow::hInvBackgroundBrush[8] = LKBrush_RifleGrey;
  MapWindow::hInvBackgroundBrush[9] = LKBrush_Black;


  extern LKColor taskcolor;
  MapWindow::hpStartFinishThick.Create(PEN_SOLID, NIBLSCALE(4), taskcolor);
  MapWindow::hpStartFinishThin.Create(PEN_SOLID, NIBLSCALE(2), taskcolor);
  MapWindow::hpWindThick.Create(PEN_SOLID, NIBLSCALE(4), LKColor(255,220,220));
  MapWindow::hpThermalBand.Create(PEN_SOLID, NIBLSCALE(2), LKColor(0x40,0x40,0xFF));
  MapWindow::hpThermalBandGlider.Create(PEN_SOLID, NIBLSCALE(2), LKColor(0x00,0x00,0x30));
  MapWindow::hpFinalGlideBelow.Create(PEN_SOLID, NIBLSCALE(1), LKColor(0xFF,0xA0,0xA0)); // another light red
  MapWindow::hpFinalGlideAbove.Create(PEN_SOLID, NIBLSCALE(1), LKColor(0xA0,0xFF,0xA0)); // another light green
  #ifdef NO_DASH_LINES
  MapWindow::hpTerrainLine.Create(PEN_SOLID, ScreenThinSize, LKColor(0x30,0x30,0x30)); // shade
  #else
  MapWindow::hpTerrainLine.Create(PEN_DASH, 1, LKColor(0x30,0x30,0x30)); // shade
  #endif
  MapWindow::hpTerrainLineBg.Create(PEN_SOLID, NIBLSCALE(2), RGB_LCDDARKGREEN); // perimeter


}


void LKObjects_Delete() {

  #if TESTBENCH
  StartupStore(_T("... LKObjects_Delete\n"));
  #endif

  // No need to delete stock objects
  LKBrush_Petrol.Release();
  LKBrush_LightGreen.Release();
  LKBrush_DarkGreen.Release();
  LKBrush_Ndark.Release();
  LKBrush_Nlight.Release();
  LKBrush_Mdark.Release();
  LKBrush_Mlight.Release();
  LKBrush_Red.Release();
  LKBrush_Yellow.Release();
  LKBrush_LightYellow.Release();
  LKBrush_Green.Release();
  LKBrush_DarkYellow2.Release();
  LKBrush_Orange.Release();
  LKBrush_Lake.Release();
  LKBrush_Blue.Release();
  LKBrush_Indigo.Release();
  LKBrush_LightGrey.Release();
  LKBrush_DarkGrey.Release();
  LKBrush_LcdGreen.Release();
  LKBrush_LcdDarkGreen.Release();
  LKBrush_Grey.Release();
  LKBrush_Emerald.Release();
  LKBrush_DarkSlate.Release();
  LKBrush_RifleGrey.Release();
  LKBrush_LightCyan.Release();

  LKBrush_Vario_neg4.Release();
  LKBrush_Vario_neg3.Release();
  LKBrush_Vario_neg2.Release();
  LKBrush_Vario_neg1.Release();
  LKBrush_Vario_0.Release();
  LKBrush_Vario_pos1.Release();
  LKBrush_Vario_pos2.Release();
  LKBrush_Vario_pos3.Release();
  LKBrush_Vario_pos4.Release();
  
  LKBrush_Higlighted.Release();
  LKBrush_FormBackGround.Release();


  LKPen_Black_N0.Release();
  LKPen_Black_N1.Release();
  LKPen_Black_N2.Release();
  LKPen_Black_N3.Release();
  LKPen_Black_N4.Release();
  LKPen_Black_N5.Release();
  LKPen_White_N0.Release();
  LKPen_White_N1.Release();
  LKPen_White_N2.Release();
  LKPen_White_N3.Release();
  LKPen_White_N4.Release();
  LKPen_White_N5.Release();

  LKPen_Petrol_C2.Release();
  LKPen_Green_N1.Release();
  LKPen_Red_N1.Release();
  LKPen_Blue_N1.Release();
  LKPen_Grey_N0.Release();
  LKPen_Grey_N1.Release();
  LKPen_Grey_N2.Release();
  LKPen_GABRG.Release();


  MapWindow::hAirspaceBorderPen.Release();

  std::for_each(std::begin(MapWindow::hSnailPens), std::end(MapWindow::hSnailPens), std::bind(&LKPen::Release, _1) );

  std::for_each(std::begin(MapWindow::hAirspacePens), std::end(MapWindow::hAirspacePens), std::bind(&LKPen::Release, _1) );
  std::for_each(std::begin(MapWindow::hBigAirspacePens), std::end(MapWindow::hBigAirspacePens), std::bind(&LKPen::Release, _1) );
  std::for_each(std::begin(MapWindow::hAirSpaceSldBrushes), std::end(MapWindow::hAirSpaceSldBrushes), std::bind(&LKBrush::Release, _1));

  std::for_each(std::begin(MapWindow::hAirspaceBrushes), std::end(MapWindow::hAirspaceBrushes), std::bind(&LKBrush::Release, _1) );
  
#ifndef ENABLE_OPENGL  
  MapWindow::hAboveTerrainBrush.Release();
#endif
  
  MapWindow::hpStartFinishThick.Release();
  MapWindow::hpStartFinishThin.Release();
  MapWindow::hpWindThick.Release();
  MapWindow::hpThermalBand.Release();
  MapWindow::hpThermalBandGlider.Release();
  MapWindow::hpFinalGlideBelow.Release();
  MapWindow::hpFinalGlideAbove.Release();
  MapWindow::hpTerrainLine.Release();
  MapWindow::hpTerrainLineBg.Release();
}




