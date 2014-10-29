/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "RGB.h"


#ifdef CPUSTATS
void MapWindow::DrawCpuStats(HDC hdc, RECT rc) {

  if (Appearance.InverseInfoBox == true) return;

  TCHAR Buffer[LKSIZEBUFFERLARGE];
  TextInBoxMode_t TextDisplayMode = {0};
  TextDisplayMode.Color = RGB_WHITE;
  TextDisplayMode.WhiteBorder = 1; // inside a white circle
  TextDisplayMode.Border = 1;      // add a black border to the circle

#if (WINDOWSPC>0)
  _stprintf(Buffer,_T("CPU Draw=%d Calc=%d us"), Cpu_Draw, Cpu_Calc);
#else
  _stprintf(Buffer,_T("CPU Draw=%d Calc=%d ms"), Cpu_Draw, Cpu_Calc);
#endif
  TextInBox(hdc, &rc, Buffer, 000, 200 , 0, &TextDisplayMode, false);
#if (WINDOWSPC>0)
  _stprintf(Buffer,_T("CPU Inst=%d PortA=%d PortB=%d us"), Cpu_Instrument, Cpu_PortA, Cpu_PortB);
#else
  _stprintf(Buffer,_T("CPU Inst=%d PortA=%d PortB=%d ms"), Cpu_Instrument, Cpu_PortA, Cpu_PortB);
#endif
  TextInBox(hdc, &rc, Buffer, 000, 240 , 0, &TextDisplayMode, false);

  //_stprintf(Buffer,_T("Landsc=%d Geom=%d"), InfoBoxLayout::landscape, InfoBoxLayout::InfoBoxGeometry);
  //TextInBox(hdc, Buffer, 000, 280 , 0, TextDisplayMode, false);
  //_stprintf(Buffer,_T("Recents=%d"), RecentNumber);
  //TextInBox(hdc, Buffer, 000, 280 , 0, TextDisplayMode, false);

}
#endif

#ifdef DRAWDEBUG
void MapWindow::DrawDebug(HDC hdc, RECT rc) {

  TCHAR Buffer[LKSIZEBUFFERLARGE];
  TextInBoxMode_t TextDisplayMode = {0};
  TextDisplayMode.Color = RGB_WHITE;
  TextDisplayMode.WhiteBorder = 1; // inside a white circle
  TextDisplayMode.Border = 1;      // add a black border to the circle

  _stprintf(Buffer,_T("ModeIndex=%d CURTYPE=%d MSM=%d"), ModeIndex, ModeType[ModeIndex],MapSpaceMode );
  TextInBox(hdc, &rc, Buffer, 000, 200 , 0, &TextDisplayMode, false);
  _stprintf(Buffer,_T("MTableTop=%d ModeTable=%d=MSM"), ModeTableTop[ModeIndex], ModeTable[ModeIndex][ModeType[ModeIndex]] );
  TextInBox(hdc, &rc, Buffer, 000, 240 , 0, &TextDisplayMode, false);

}
#endif



#ifdef DRAWLKSTATUS
// LK Status message
void MapWindow::DrawLKStatus(HDC hdc, RECT rc) {

  TextInBoxMode_t TextDisplayMode = {0};
  TCHAR Buffer[LKSIZEBUFFERLARGE];

  short bottomlines;
  short middlex=(rc.right-rc.left)/2;
  short left=rc.left+NIBLSCALE(5);
  short contenttop=rc.top+NIBLSCALE(50);

  TextDisplayMode.Color = RGB_BLACK;
  TextDisplayMode.NoSetFont = 1; 
  //TextDisplayMode.AlligneRight = 0;
  TextDisplayMode.AlligneCenter = 1;
  TextDisplayMode.WhiteBold = 1;
  TextDisplayMode.Border = 1;
  // HFONT oldfont=(HFONT)Surface.SelectObject(LK8PanelBigFont);

  switch(ModeIndex) {
	case LKMODE_MAP:
		_stprintf(Buffer,TEXT("MAP mode, 1 of 1"));
		break;
	case LKMODE_INFOMODE:
		_stprintf(Buffer,TEXT("%d-%d"), ModeIndex,CURTYPE+1);
		break;
	case LKMODE_WP:
		_stprintf(Buffer,TEXT("%d-%d"), ModeIndex,CURTYPE+1);
		break;
	case LKMODE_NAV:
		_stprintf(Buffer,TEXT("%d-%d"), ModeIndex,CURTYPE+1);
		break;
	default:
		_stprintf(Buffer,TEXT("UNKOWN mode"));
		break;
  }
  TextInBox(hdc, &rc, Buffer, middlex, 200 , 0, &TextDisplayMode, false);

  //Surface.SelectObject(oldfont);
  return;
}
#endif


