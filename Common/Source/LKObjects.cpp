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

#include "RGB.h"

#include "utils/stl_utils.h"
using std::placeholders::_1;

void LKObjects_Create() {

  #if TESTBENCH
  StartupStore(_T("... LKObjects_Create\n"));
  #endif

  // CUSTOM BRUSHES
  LKBrush_Petrol.Create(RGB_PETROL);
  LKBrush_LightGreen.Create(RGB_LIGHTGREEN);
  LKBrush_DarkGreen.Create(RGB_DARKGREEN);
  LKBrush_Ndark.Create(RGB_NDARK);
  LKBrush_Nlight.Create(RGB_NLIGHT);
  LKBrush_Mdark.Create(RGB_MDARK);
  LKBrush_Mlight.Create(RGB_MLIGHT);
  LKBrush_Red.Create(RGB_RED);
  LKBrush_Yellow.Create(RGB_YELLOW);
  LKBrush_LightYellow.Create(RGB_LIGHTYELLOW);
  LKBrush_Green.Create(RGB_GREEN);
  LKBrush_DarkYellow2.Create(RGB_DARKYELLOW2);
  LKBrush_Orange.Create(RGB_ORANGE);
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
  LKPen_Black_N0.Create(PEN_SOLID,1,RGB_BLACK);
  LKPen_Black_N1.Create(PEN_SOLID,NIBLSCALE(1),RGB_BLACK);
  LKPen_Black_N2.Create(PEN_SOLID,NIBLSCALE(2),RGB_BLACK);
  LKPen_Black_N3.Create(PEN_SOLID,NIBLSCALE(3),RGB_BLACK);
  LKPen_Black_N4.Create(PEN_SOLID,NIBLSCALE(4),RGB_BLACK);
  LKPen_Black_N5.Create(PEN_SOLID,NIBLSCALE(5),RGB_BLACK);

  LKPen_White_N0.Create(PEN_SOLID,1,RGB_WHITE);
  LKPen_White_N1.Create(PEN_SOLID,NIBLSCALE(1),RGB_WHITE);
  LKPen_White_N2.Create(PEN_SOLID,NIBLSCALE(2),RGB_WHITE);
  LKPen_White_N3.Create(PEN_SOLID,NIBLSCALE(3),RGB_WHITE);
  LKPen_White_N4.Create(PEN_SOLID,NIBLSCALE(4),RGB_WHITE);
  LKPen_White_N5.Create(PEN_SOLID,NIBLSCALE(5),RGB_WHITE);

  LKPen_Petrol_C2.Create(PEN_SOLID,NIBLSCALE(1)+2,RGB_PETROL);

  LKPen_Green_N1.Create(PEN_SOLID,NIBLSCALE(1),RGB_GREEN);
  LKPen_Red_N1.Create(PEN_SOLID,NIBLSCALE(1),RGB_RED);
  LKPen_Blue_N1.Create(PEN_SOLID,NIBLSCALE(1),RGB_BLUE);

  LKPen_Grey_N0.Create(PEN_SOLID,1,RGB_GREY);
  LKPen_Grey_N1.Create(PEN_SOLID,NIBLSCALE(1),RGB_GREY);
  LKPen_Grey_N2.Create(PEN_SOLID,NIBLSCALE(2),RGB_GREY);

#ifdef __linux__

  MapWindow::hpAircraft = LKPen_Black_N3; 
  LKPen_GABRG.Create(PEN_SOLID,NIBLSCALE(3),RGB_MAGENTA);

#else
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
#endif

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

  int iwidth;
  iwidth=IBLSCALE(MapWindow::SnailWidthScale);
  MapWindow::hSnailColours[0] = RGB_BLACK;
  MapWindow::hSnailColours[1] = RGB_INDIGO;
  MapWindow::hSnailColours[2] = RGB_INDIGO;
  MapWindow::hSnailColours[3] = RGB_BLUE;
  MapWindow::hSnailColours[4] = RGB_BLUE;
  MapWindow::hSnailColours[5] = RGB_LAKE;
  MapWindow::hSnailColours[6] = RGB_LAKE;
  MapWindow::hSnailColours[7] = RGB_GREY;
  MapWindow::hSnailColours[8] = RGB_GREEN;
  MapWindow::hSnailColours[9] = RGB_GREEN;
  MapWindow::hSnailColours[10] = RGB_ORANGE;
  MapWindow::hSnailColours[11] = RGB_ORANGE;
  MapWindow::hSnailColours[12] = RGB_RED;
  MapWindow::hSnailColours[13] = RGB_RED;
  MapWindow::hSnailColours[14] = RGB_DARKRED;

  MapWindow::hSnailPens[0].Create(PEN_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[0]);
  MapWindow::hSnailPens[1].Create(PEN_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[1]);
  MapWindow::hSnailPens[2].Create(PEN_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[2]);
  MapWindow::hSnailPens[3].Create(PEN_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[3]);
  MapWindow::hSnailPens[4].Create(PEN_SOLID,  iwidth/NIBLSCALE(2), MapWindow::hSnailColours[4]);
  MapWindow::hSnailPens[5].Create(PEN_SOLID,  iwidth/NIBLSCALE(4), MapWindow::hSnailColours[5]);
  MapWindow::hSnailPens[6].Create(PEN_SOLID,  iwidth/NIBLSCALE(4), MapWindow::hSnailColours[6]);
  MapWindow::hSnailPens[7].Create(PEN_SOLID,  iwidth/NIBLSCALE(6), MapWindow::hSnailColours[7]);
  MapWindow::hSnailPens[8].Create(PEN_SOLID,  iwidth/NIBLSCALE(4), MapWindow::hSnailColours[8]);
  MapWindow::hSnailPens[9].Create(PEN_SOLID,  iwidth/NIBLSCALE(4), MapWindow::hSnailColours[9]);
  MapWindow::hSnailPens[10].Create(PEN_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[10]);
  MapWindow::hSnailPens[11].Create(PEN_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[11]);
  MapWindow::hSnailPens[12].Create(PEN_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[12]);
  MapWindow::hSnailPens[13].Create(PEN_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[13]);
  MapWindow::hSnailPens[14].Create(PEN_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[14]);


  for (unsigned i=0; i<array_size(MapWindow::hAirspaceBrushes); ++i) {
#ifdef HAVE_HATCHED_BRUSH
      static_assert(array_size(MapWindow::hAirspaceBrushes) == array_size(hAirspaceBitmap), "Array Size error");
      MapWindow::hAirspaceBrushes[i].Create(hAirspaceBitmap[i]);
#else
      static_assert(array_size(MapWindow::hAirspaceBrushes) == array_size(MapWindow::Colours), "Array Size error");
      MapWindow::hAirspaceBrushes[i].Create(MapWindow::Colours[i].WithAlpha(0xFF/2));
#endif
  }

#ifdef HAVE_HATCHED_BRUSH
  MapWindow::hAboveTerrainBrush.Create(hAboveTerrainBitmap);
#else
  MapWindow::hAboveTerrainBrush.Create(RGB_GREY);
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
  MapWindow::hpStartFinishThick.Create(PEN_SOLID, NIBLSCALE(2)+1, taskcolor);
  MapWindow::hpMapScale2.Create(PEN_SOLID, NIBLSCALE(1)+1, RGB_BLACK);
  MapWindow::hpWindThick.Create(PEN_SOLID, NIBLSCALE(4), LKColor(255,220,220));
  MapWindow::hpThermalBand.Create(PEN_SOLID, NIBLSCALE(2), LKColor(0x40,0x40,0xFF));
  MapWindow::hpThermalBandGlider.Create(PEN_SOLID, NIBLSCALE(2), LKColor(0x00,0x00,0x30));
  MapWindow::hpFinalGlideBelow.Create(PEN_SOLID, NIBLSCALE(1), LKColor(0xFF,0xA0,0xA0)); // another light red
  MapWindow::hpFinalGlideAbove.Create(PEN_SOLID, NIBLSCALE(1), LKColor(0xA0,0xFF,0xA0)); // another light green
  MapWindow::hpTerrainLine.Create(PEN_DASH, (1), LKColor(0x30,0x30,0x30)); // shade
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
  
  MapWindow::hAboveTerrainBrush.Release();
  
  MapWindow::hpStartFinishThick.Release();
  MapWindow::hpMapScale2.Release();
  MapWindow::hpWindThick.Release();
  MapWindow::hpThermalBand.Release();
  MapWindow::hpThermalBandGlider.Release();
  MapWindow::hpFinalGlideBelow.Release();
  MapWindow::hpFinalGlideAbove.Release();
  MapWindow::hpTerrainLine.Release();
  MapWindow::hpTerrainLineBg.Release();
}




