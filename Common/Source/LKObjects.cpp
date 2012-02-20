/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: LKObjects.cpp,v 1.1 2010/12/11 19:21:43 root Exp root $
*/

//  Create common shared graphic objects, from MapWindow

#include "externs.h"
#include "Utils.h"
#if (WINDOWSPC>0)
#include <wingdi.h>
#endif

#define STATIC_LKOBJECTS
#include "LKObjects.h"

#include "RGB.h"


void LKObjects_Create() {

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
  LKBrush_LcdGreen = CreateSolidBrush(COLORREF RGB_LCDGREEN);
  LKBrush_LcdDarkGreen = CreateSolidBrush(COLORREF RGB_LCDDARKGREEN);
  LKBrush_Grey = CreateSolidBrush(COLORREF RGB_GREY);
  LKBrush_Emerald = CreateSolidBrush(COLORREF RGB_EMERALD);
  LKBrush_DarkSlate = CreateSolidBrush(COLORREF RGB_DARKSLATE);
  LKBrush_LightCyan = CreateSolidBrush(COLORREF RGB_LIGHTCYAN);
  LKBrush_RifleGrey = CreateSolidBrush(COLORREF RGB_RIFLEGREY);

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
  LKPen_GABRG = (HPEN) CreatePen(PS_SOLID,NIBLSCALE(3),RGB_MAGENTA);


  //
  // MapWindow objects
  //

  for (int i=0; i<AIRSPACECLASSCOUNT; i++) {
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

}


void LKObjects_Delete() {

  // No need to delete stock objects
  DeleteObject(LKBrush_Petrol);
  DeleteObject(LKBrush_LightGreen);
  DeleteObject(LKBrush_DarkGreen);
  DeleteObject(LKBrush_Ndark);
  DeleteObject(LKBrush_Nlight);
  DeleteObject(LKBrush_Mdark);
  DeleteObject(LKBrush_Mlight);
  DeleteObject(LKBrush_Red);
  DeleteObject(LKBrush_Yellow);
  DeleteObject(LKBrush_Green);
  DeleteObject(LKBrush_DarkYellow2);
  DeleteObject(LKBrush_Orange);
  DeleteObject(LKBrush_Lake);
  DeleteObject(LKBrush_Blue);
  DeleteObject(LKBrush_Indigo);
  DeleteObject(LKBrush_LightGrey);
  DeleteObject(LKBrush_LcdGreen);
  DeleteObject(LKBrush_LcdDarkGreen);
  DeleteObject(LKBrush_Grey);
  DeleteObject(LKBrush_Emerald);
  DeleteObject(LKBrush_DarkSlate);
  DeleteObject(LKBrush_RifleGrey);
  DeleteObject(LKBrush_LightCyan);

  DeleteObject(LKPen_Black_N0);
  DeleteObject(LKPen_Black_N1);
  DeleteObject(LKPen_Black_N2);
  DeleteObject(LKPen_Black_N3);
  DeleteObject(LKPen_Black_N4);
  DeleteObject(LKPen_Black_N5);
  DeleteObject(LKPen_White_N0);
  DeleteObject(LKPen_White_N1);
  DeleteObject(LKPen_White_N2);
  DeleteObject(LKPen_White_N3);
  DeleteObject(LKPen_White_N4);
  DeleteObject(LKPen_White_N5);
  DeleteObject(LKPen_Green_N1);
  DeleteObject(LKPen_Red_N1);
  DeleteObject(LKPen_Blue_N1);
  DeleteObject(LKPen_Grey_N1);
  DeleteObject(LKPen_Grey_N2);
  DeleteObject(LKPen_Petrol_C2);
  DeleteObject(LKPen_GABRG);

  int i;

  for (i=0; i<AIRSPACECLASSCOUNT; i++) {
	DeleteObject(MapWindow::hAirspacePens[i]);
  }
  DeleteObject(MapWindow::hAirspaceBorderPen);

  for (i=0; i<NUMSNAILCOLORS; i++) {
	DeleteObject(MapWindow::hSnailPens[i]);
  }





}

