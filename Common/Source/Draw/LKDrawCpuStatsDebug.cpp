/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"


#ifdef CPUSTATS
void MapWindow::DrawCpuStats(HDC hdc, RECT rc) {

  if (Appearance.InverseInfoBox == true) return;

  TCHAR Buffer[LKSIZEBUFFERLARGE];
  TextInBoxMode_t TextDisplayMode;
  TextDisplayMode.AsInt = 0;
  TextDisplayMode.AsFlag.Color = TEXTWHITE;
  TextDisplayMode.AsFlag.WhiteBorder = 1; // inside a white circle
  TextDisplayMode.AsFlag.Border = 1;      // add a black border to the circle

#if (WINDOWSPC>0)
  wsprintf(Buffer,_T("CPU Draw=%d Calc=%d us"), Cpu_Draw, Cpu_Calc);
#else
  wsprintf(Buffer,_T("CPU Draw=%d Calc=%d ms"), Cpu_Draw, Cpu_Calc);
#endif
  TextInBox(hdc, Buffer, 000, 200 , 0, TextDisplayMode, false);
#if (WINDOWSPC>0)
  wsprintf(Buffer,_T("CPU Inst=%d Port=%d us"), Cpu_Instrument, Cpu_Port);
#else
  wsprintf(Buffer,_T("CPU Inst=%d Port=%d ms"), Cpu_Instrument, Cpu_Port);
#endif
  TextInBox(hdc, Buffer, 000, 240 , 0, TextDisplayMode, false);

  //wsprintf(Buffer,_T("Landsc=%d Geom=%d"), InfoBoxLayout::landscape, InfoBoxLayout::InfoBoxGeometry);
  //TextInBox(hdc, Buffer, 000, 280 , 0, TextDisplayMode, false);
  //wsprintf(Buffer,_T("Recents=%d"), RecentNumber);
  //TextInBox(hdc, Buffer, 000, 280 , 0, TextDisplayMode, false);

}
#endif

#ifdef DRAWDEBUG
void MapWindow::DrawDebug(HDC hdc, RECT rc) {

  TCHAR Buffer[LKSIZEBUFFERLARGE];
  TextInBoxMode_t TextDisplayMode;
  TextDisplayMode.AsInt = 0;
  TextDisplayMode.AsFlag.Color = TEXTWHITE;
  TextDisplayMode.AsFlag.WhiteBorder = 1; // inside a white circle
  TextDisplayMode.AsFlag.Border = 1;      // add a black border to the circle

  wsprintf(Buffer,_T("ModeIndex=%d CURTYPE=%d MSM=%d"), ModeIndex, ModeType[ModeIndex],MapSpaceMode );
  TextInBox(hdc, Buffer, 000, 200 , 0, TextDisplayMode, false);
  wsprintf(Buffer,_T("MTableTop=%d ModeTable=%d=MSM"), ModeTableTop[ModeIndex], ModeTable[ModeIndex][ModeType[ModeIndex]] );
  TextInBox(hdc, Buffer, 000, 240 , 0, TextDisplayMode, false);

}
#endif


