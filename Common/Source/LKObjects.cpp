/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKObjects.cpp,v 1.1 2010/12/11 19:21:43 root Exp root $
*/

//  Create common shared graphic objects, from MapWindow

#include "externs.h"
#if (WINDOWSPC>0)
#include <wingdi.h>
#endif

#define STATIC_LKOBJECTS
#include "LKObjects.h"
#include "Bitmaps.h"

#include "RGB.h"

extern COLORREF ChangeBrightness(long Color, double fBrightFact);


void LKObjects_Create() {

  #if TESTBENCH
  StartupStore(_T("... LKObjects_Create\n"));
  #endif

  // STOCK BRUSHES
  LKBrush_White = (HBRUSH)GetStockObject(WHITE_BRUSH);
  LKBrush_Black = (HBRUSH)GetStockObject(BLACK_BRUSH);

  // CUSTOM BRUSHES
  LKBrush_Petrol = CreateSolidBrush(COLORREF RGB_PETROL);
  LKBrush_LightGreen = CreateSolidBrush(COLORREF RGB_LIGHTGREEN);
  LKBrush_DarkGreen = CreateSolidBrush(COLORREF RGB_DARKGREEN);
  LKBrush_Ndark = CreateSolidBrush(COLORREF RGB_NDARK);
  LKBrush_Nlight = CreateSolidBrush(COLORREF RGB_NLIGHT);
  LKBrush_Mdark = CreateSolidBrush(COLORREF RGB_MDARK);
  LKBrush_Mlight = CreateSolidBrush(COLORREF RGB_MLIGHT);
  LKBrush_Red = CreateSolidBrush(COLORREF RGB_RED);
  LKBrush_Yellow = CreateSolidBrush(COLORREF RGB_YELLOW);
  LKBrush_Green = CreateSolidBrush(COLORREF RGB_GREEN);
  LKBrush_DarkYellow2 = CreateSolidBrush(COLORREF RGB_DARKYELLOW2);
  LKBrush_Orange = CreateSolidBrush(COLORREF RGB_ORANGE);
  LKBrush_Lake = CreateSolidBrush(COLORREF RGB_LAKE);
  LKBrush_Blue = CreateSolidBrush(COLORREF RGB_BLUE);
  LKBrush_Indigo = CreateSolidBrush(COLORREF RGB_INDIGO);
  LKBrush_LightGrey = CreateSolidBrush(COLORREF RGB_LIGHTGREY);
  LKBrush_DarkGrey = CreateSolidBrush(RGB(100,100,100));
  LKBrush_LcdGreen = CreateSolidBrush(COLORREF RGB_LCDGREEN);
  LKBrush_LcdDarkGreen = CreateSolidBrush(COLORREF RGB_LCDDARKGREEN);
  LKBrush_Grey = CreateSolidBrush(COLORREF RGB_GREY);
  LKBrush_Emerald = CreateSolidBrush(COLORREF RGB_EMERALD);
  LKBrush_DarkSlate = CreateSolidBrush(COLORREF RGB_DARKSLATE);
  LKBrush_LightCyan = CreateSolidBrush(COLORREF RGB_LIGHTCYAN);
  LKBrush_RifleGrey = CreateSolidBrush(COLORREF RGB_RIFLEGREY);

  LKBrush_Vario_neg4 = CreateSolidBrush(ChangeBrightness(RGB_BLUE, 0.4));
  LKBrush_Vario_neg3 = CreateSolidBrush(ChangeBrightness(RGB_BLUE, 0.6));
  LKBrush_Vario_neg2 = CreateSolidBrush(ChangeBrightness(RGB_BLUE, 0.8));
  LKBrush_Vario_neg1 = CreateSolidBrush(ChangeBrightness(RGB_BLUE, 1.0));
  LKBrush_Vario_0    = CreateSolidBrush(ChangeBrightness(RGB_YELLOW, 0.8));
  LKBrush_Vario_pos1 = CreateSolidBrush(ChangeBrightness(RGB_GREEN, 0.6));
  LKBrush_Vario_pos2 = CreateSolidBrush(ChangeBrightness(RGB_GREEN, 0.7));
  LKBrush_Vario_pos3 = CreateSolidBrush(ChangeBrightness(RGB_GREEN, 0.8));
  LKBrush_Vario_pos4 = CreateSolidBrush(ChangeBrightness(RGB_GREEN, 1.0));
  // CUSTOM PENS
  LKPen_Black_N0 = (HPEN) CreatePen(PS_SOLID,0,RGB_BLACK);
  LKPen_Black_N1 = (HPEN) CreatePen(PS_SOLID,NIBLSCALE(1),RGB_BLACK);
  LKPen_Black_N2 = (HPEN) CreatePen(PS_SOLID,NIBLSCALE(2),RGB_BLACK);
  LKPen_Black_N3= (HPEN) CreatePen(PS_SOLID,NIBLSCALE(3),RGB_BLACK);
  LKPen_Black_N4= (HPEN) CreatePen(PS_SOLID,NIBLSCALE(4),RGB_BLACK);
  LKPen_Black_N5= (HPEN) CreatePen(PS_SOLID,NIBLSCALE(5),RGB_BLACK);

  LKPen_White_N0= (HPEN) CreatePen(PS_SOLID,0,RGB_WHITE);
  LKPen_White_N1= (HPEN) CreatePen(PS_SOLID,NIBLSCALE(1),RGB_WHITE);
  LKPen_White_N2= (HPEN) CreatePen(PS_SOLID,NIBLSCALE(2),RGB_WHITE);
  LKPen_White_N3= (HPEN) CreatePen(PS_SOLID,NIBLSCALE(3),RGB_WHITE);
  LKPen_White_N4= (HPEN) CreatePen(PS_SOLID,NIBLSCALE(4),RGB_WHITE);
  LKPen_White_N5= (HPEN) CreatePen(PS_SOLID,NIBLSCALE(5),RGB_WHITE);

  LKPen_Petrol_C2= (HPEN) CreatePen(PS_SOLID,NIBLSCALE(1)+2,RGB_PETROL);

  LKPen_Green_N1 = (HPEN) CreatePen(PS_SOLID,NIBLSCALE(1),RGB_GREEN);
  LKPen_Red_N1 = (HPEN) CreatePen(PS_SOLID,NIBLSCALE(1),RGB_RED);
  LKPen_Blue_N1 = (HPEN) CreatePen(PS_SOLID,NIBLSCALE(1),RGB_BLUE);

  LKPen_Grey_N1 = (HPEN) CreatePen(PS_SOLID,NIBLSCALE(1),RGB_GREY);
  LKPen_Grey_N2 = (HPEN) CreatePen(PS_SOLID,NIBLSCALE(2),RGB_GREY);


  switch(ScreenSize) {
	// portrait small screen
	case ss240x320:
	case ss240x400:
	case ss272x480:
	// landscape small screen
	case ss320x240:
	case ss400x240:
	case ss480x272:
		LKPen_GABRG = (HPEN) CreatePen(PS_SOLID,NIBLSCALE(5),RGB_MAGENTA);
		break;
	default:
		LKPen_GABRG = (HPEN) CreatePen(PS_SOLID,NIBLSCALE(3),RGB_MAGENTA);
		break;
  }

  //
  // MapWindow objects
  //
  int i;

  for (i=0; i<AIRSPACECLASSCOUNT; i++) {
	MapWindow::hAirspacePens[i] = CreatePen(PS_SOLID, NIBLSCALE(2), MapWindow::Colours[MapWindow::iAirspaceColour[i]]);
  }
  MapWindow::hAirspaceBorderPen = CreatePen(PS_SOLID, NIBLSCALE(10), RGB_WHITE);

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

  MapWindow::hSnailPens[0] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[0]);
  MapWindow::hSnailPens[1] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[1]);
  MapWindow::hSnailPens[2] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[2]);
  MapWindow::hSnailPens[3] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[3]);
  MapWindow::hSnailPens[4] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(2), MapWindow::hSnailColours[4]);
  MapWindow::hSnailPens[5] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(4), MapWindow::hSnailColours[5]);
  MapWindow::hSnailPens[6] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(4), MapWindow::hSnailColours[6]);
  MapWindow::hSnailPens[7] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(6), MapWindow::hSnailColours[7]);
  MapWindow::hSnailPens[8] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(4), MapWindow::hSnailColours[8]);
  MapWindow::hSnailPens[9] = (HPEN)CreatePen(PS_SOLID,  iwidth/NIBLSCALE(4), MapWindow::hSnailColours[9]);
  MapWindow::hSnailPens[10] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[10]);
  MapWindow::hSnailPens[11] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[11]);
  MapWindow::hSnailPens[12] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[12]);
  MapWindow::hSnailPens[13] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[13]);
  MapWindow::hSnailPens[14] = (HPEN)CreatePen(PS_SOLID, iwidth/NIBLSCALE(2), MapWindow::hSnailColours[14]);

  for (i=0; i<NUMAIRSPACEBRUSHES; i++) {
	MapWindow::hAirspaceBrushes[i] = CreatePatternBrush((HBITMAP)hAirspaceBitmap[i]);
  }
  MapWindow::hAboveTerrainBrush = CreatePatternBrush((HBITMAP)hAboveTerrainBitmap);

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

  MapWindow::hpCompassBorder = LKPen_Black_N2;
  MapWindow::hpAircraft = LKPen_White_N3;
  MapWindow::hpAircraftBorder = LKPen_Black_N1;
  MapWindow::hpWind = LKPen_Black_N2;
  MapWindow::hpBearing = LKPen_GABRG;
  MapWindow::hpBestCruiseTrack = LKPen_Blue_N1;

  extern COLORREF taskcolor;
  MapWindow::hpStartFinishThick=(HPEN)CreatePen(PS_SOLID, NIBLSCALE(2)+1, taskcolor);
  MapWindow::hpMapScale2 = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1)+1, RGB_BLACK);
  MapWindow::hpWindThick = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(4), RGB(255,220,220));
  MapWindow::hpCompass = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB_BLACK);
  MapWindow::hpThermalBand = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB(0x40,0x40,0xFF));
  MapWindow::hpThermalBandGlider = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB(0x00,0x00,0x30));
  MapWindow::hpFinalGlideBelow = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB(0xFF,0xA0,0xA0)); // another light red
  MapWindow::hpFinalGlideAbove = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(1), RGB(0xA0,0xFF,0xA0)); // another light green
  MapWindow::hpTerrainLine = (HPEN)CreatePen(PS_DASH, (1), RGB(0x30,0x30,0x30)); // shade
  MapWindow::hpTerrainLineBg = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB_LCDDARKGREEN); // perimeter
#ifdef GTL2
  MapWindow::hpTerrainLine2Bg = (HPEN)CreatePen(PS_SOLID, NIBLSCALE(2), RGB_WHITE);
#endif
  MapWindow::hpVisualGlideLightBlack = (HPEN)CreatePen(PS_DASH, (1), RGB_BLACK);
  MapWindow::hpVisualGlideHeavyBlack = (HPEN)CreatePen(PS_DASH, (2), RGB_BLACK);
  MapWindow::hpVisualGlideLightRed = (HPEN)CreatePen(PS_DASH, (1), RGB_RED);
  MapWindow::hpVisualGlideHeavyRed = (HPEN)CreatePen(PS_DASH, (2), RGB_RED);


  MapWindow::hpStartFinishThin=LKPen_Red_N1;
  MapWindow::hpMapScale = LKPen_Black_N1;
  MapWindow::hbThermalBand=LKBrush_Emerald;
  MapWindow::hbCompass=LKBrush_White;
  MapWindow::hbBestCruiseTrack=LKBrush_Blue;
  MapWindow::hbFinalGlideBelow=LKBrush_Red;
  MapWindow::hbFinalGlideAbove=LKBrush_Green;
  MapWindow::hbWind=LKBrush_Grey;


}


void LKObjects_Delete() {

  #if TESTBENCH
  StartupStore(_T("... LKObjects_Delete\n"));
  #endif

  // No need to delete stock objects
  if(LKBrush_Petrol) DeleteObject(LKBrush_Petrol);
  if(LKBrush_LightGreen) DeleteObject(LKBrush_LightGreen);
  if(LKBrush_DarkGreen) DeleteObject(LKBrush_DarkGreen);
  if(LKBrush_Ndark) DeleteObject(LKBrush_Ndark);
  if(LKBrush_Nlight) DeleteObject(LKBrush_Nlight);
  if(LKBrush_Mdark) DeleteObject(LKBrush_Mdark);
  if(LKBrush_Mlight) DeleteObject(LKBrush_Mlight);
  if(LKBrush_Red) DeleteObject(LKBrush_Red);
  if(LKBrush_Yellow) DeleteObject(LKBrush_Yellow);
  if(LKBrush_Green) DeleteObject(LKBrush_Green);
  if(LKBrush_DarkYellow2) DeleteObject(LKBrush_DarkYellow2);
  if(LKBrush_Orange) DeleteObject(LKBrush_Orange);
  if(LKBrush_Lake) DeleteObject(LKBrush_Lake);
  if(LKBrush_Blue) DeleteObject(LKBrush_Blue);
  if(LKBrush_Indigo) DeleteObject(LKBrush_Indigo);
  if(LKBrush_LightGrey) DeleteObject(LKBrush_LightGrey);
  if(LKBrush_DarkGrey) DeleteObject(LKBrush_DarkGrey);
  if(LKBrush_LcdGreen) DeleteObject(LKBrush_LcdGreen);
  if(LKBrush_LcdDarkGreen) DeleteObject(LKBrush_LcdDarkGreen);
  if(LKBrush_Grey) DeleteObject(LKBrush_Grey);
  if(LKBrush_Emerald) DeleteObject(LKBrush_Emerald);
  if(LKBrush_DarkSlate) DeleteObject(LKBrush_DarkSlate);
  if(LKBrush_RifleGrey) DeleteObject(LKBrush_RifleGrey);
  if(LKBrush_LightCyan) DeleteObject(LKBrush_LightCyan);

  if(LKBrush_Vario_neg4) DeleteObject(LKBrush_Vario_neg4);
  if(LKBrush_Vario_neg3) DeleteObject(LKBrush_Vario_neg3);
  if(LKBrush_Vario_neg2) DeleteObject(LKBrush_Vario_neg2);
  if(LKBrush_Vario_neg1) DeleteObject(LKBrush_Vario_neg1);
  if(LKBrush_Vario_0)    DeleteObject(LKBrush_Vario_0);
  if(LKBrush_Vario_pos1) DeleteObject(LKBrush_Vario_pos1);
  if(LKBrush_Vario_pos2) DeleteObject(LKBrush_Vario_pos2);
  if(LKBrush_Vario_pos3) DeleteObject(LKBrush_Vario_pos3);
  if(LKBrush_Vario_pos4) DeleteObject(LKBrush_Vario_pos4);


  if(LKPen_Black_N0) DeleteObject(LKPen_Black_N0);
  if(LKPen_Black_N1) DeleteObject(LKPen_Black_N1);
  if(LKPen_Black_N2) DeleteObject(LKPen_Black_N2);
  if(LKPen_Black_N3) DeleteObject(LKPen_Black_N3);
  if(LKPen_Black_N4) DeleteObject(LKPen_Black_N4);
  if(LKPen_Black_N5) DeleteObject(LKPen_Black_N5);
  if(LKPen_White_N0) DeleteObject(LKPen_White_N0);
  if(LKPen_White_N1) DeleteObject(LKPen_White_N1);
  if(LKPen_White_N2) DeleteObject(LKPen_White_N2);
  if(LKPen_White_N3) DeleteObject(LKPen_White_N3);
  if(LKPen_White_N4) DeleteObject(LKPen_White_N4);
  if(LKPen_White_N5) DeleteObject(LKPen_White_N5);

  if(LKPen_Petrol_C2) DeleteObject(LKPen_Petrol_C2);
  if(LKPen_Green_N1) DeleteObject(LKPen_Green_N1);
  if(LKPen_Red_N1) DeleteObject(LKPen_Red_N1);
  if(LKPen_Blue_N1) DeleteObject(LKPen_Blue_N1);
  if(LKPen_Grey_N1) DeleteObject(LKPen_Grey_N1);
  if(LKPen_Grey_N2) DeleteObject(LKPen_Grey_N2);
  if(LKPen_GABRG) DeleteObject(LKPen_GABRG);

  int i;

  for (i=0; i<AIRSPACECLASSCOUNT; i++) {
	if(MapWindow::hAirspacePens[i]) DeleteObject(MapWindow::hAirspacePens[i]);
  }
  DeleteObject(MapWindow::hAirspaceBorderPen);

  for (i=0; i<NUMSNAILCOLORS; i++) {
	if (MapWindow::hSnailPens[i]) DeleteObject(MapWindow::hSnailPens[i]);
  }

  for(i=0;i<NUMAIRSPACEBRUSHES;i++) {
	if (MapWindow::hAirspaceBrushes[i]) DeleteObject(MapWindow::hAirspaceBrushes[i]);
  }
  if (MapWindow::hAboveTerrainBrush) DeleteObject(MapWindow::hAboveTerrainBrush);
  if (MapWindow::hpStartFinishThick) DeleteObject((HPEN)MapWindow::hpStartFinishThick);
  if (MapWindow::hpMapScale2) DeleteObject((HPEN)MapWindow::hpMapScale2);
  if (MapWindow::hpWindThick) DeleteObject((HPEN)MapWindow::hpWindThick);
  if (MapWindow::hpCompass) DeleteObject((HPEN)MapWindow::hpCompass);
  if (MapWindow::hpThermalBand) DeleteObject((HPEN)MapWindow::hpThermalBand);
  if (MapWindow::hpThermalBandGlider) DeleteObject((HPEN)MapWindow::hpThermalBandGlider);
  if (MapWindow::hpFinalGlideBelow) DeleteObject((HPEN)MapWindow::hpFinalGlideBelow);
  if (MapWindow::hpFinalGlideAbove) DeleteObject((HPEN)MapWindow::hpFinalGlideAbove);
  if (MapWindow::hpTerrainLine) DeleteObject((HPEN)MapWindow::hpTerrainLine);
  if (MapWindow::hpTerrainLineBg) DeleteObject((HPEN)MapWindow::hpTerrainLineBg);
#ifdef GTL2
  if (MapWindow::hpTerrainLine2Bg) DeleteObject((HPEN)MapWindow::hpTerrainLine2Bg);
#endif
  if (MapWindow::hpVisualGlideLightBlack) DeleteObject((HPEN)MapWindow::hpVisualGlideLightBlack);
  if (MapWindow::hpVisualGlideHeavyBlack) DeleteObject((HPEN)MapWindow::hpVisualGlideHeavyBlack);
  if (MapWindow::hpVisualGlideLightRed) DeleteObject((HPEN)MapWindow::hpVisualGlideLightRed);
  if (MapWindow::hpVisualGlideHeavyRed) DeleteObject((HPEN)MapWindow::hpVisualGlideHeavyRed);


}




