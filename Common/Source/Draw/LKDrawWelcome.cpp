/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "Logger.h"
#include "Dialogs.h"
#include "RGB.h"
#include "OS/Memory.h"

void MapWindow::DrawWelcome8000(LKSurface& Surface, const RECT& rc) {

  SIZE textSize;
  TCHAR Buffer[LKSIZEBUFFERLARGE];

  short bottomlines;
  short centerx=(rc.right+rc.left)/2;

  static double freeram=CheckFreeRam()/1000000.0;

  switch (LKevent) {
	case LKEVENT_NONE:
		break;
	case LKEVENT_ENTER:
		// Event are cleared from called inner functions, but we do it nevertheless..
		SetModeType(LKMODE_MAP, MP_MOVING);
		LKevent=LKEVENT_NONE; // check if removable 
		break;
	default:
		LKevent=LKEVENT_NONE;
		break;
  }

  int y=rc.top+NIBLSCALE(2);
  Surface.SelectObject(LK8MediumFont);

  #ifndef LKCOMPETITION
  _stprintf(Buffer,TEXT("%s v%s.%s"),_T(LKFORK),_T(LKVERSION),_T(LKRELEASE));
  #else
  _stprintf(Buffer,TEXT("%sC v%s.%s COMPETITION"),_T(LKFORK),_T(LKVERSION),_T(LKRELEASE));
  #endif
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &textSize);
  y+=(textSize.cy)/2;
  if (SIMMODE) _tcscat(Buffer,_T(" (Simulator)"));
  LKWriteText(Surface, Buffer, centerx, y , 0, WTMODE_OUTLINED, WTALIGN_CENTER,RGB_WHITENOREV, false);

  _tcscpy(Buffer,gettext(TEXT("_@M904_"))); // Tactical Flight Computer
  y+=(textSize.cy);
  LKWriteText(Surface, Buffer, centerx,y , 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_WHITENOREV, false);

  Surface.SelectObject(LK8GenericVar02Font);
  _stprintf(Buffer,TEXT("%s"), LK8000_Version);
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &textSize);
  bottomlines=rc.bottom-BottomSize-(textSize.cy*2);
  LKWriteText(Surface, Buffer, centerx, bottomlines , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITE, false);
  _stprintf(Buffer,TEXT("HTTP://WWW.LK8000.ORG  email:info@lk8000.it"));
  LKWriteText(Surface, Buffer, centerx, bottomlines+textSize.cy , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITENOREV, false);

  Surface.SelectObject(LK8InfoSmallFont);

  _stprintf(Buffer, _T("%u WPs, %0.1fM free"),(unsigned int)(WayPointList.size()-NUMRESWP),freeram);
  if (PGNumberOfGates>0) _tcscat(Buffer,_T(" (+Tsk Gates)"));
  #ifndef NDEBUG
  _tcscat(Buffer,_T(" (+debug)"));
  #endif
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &textSize);
  LKWriteText(Surface, Buffer, centerx, bottomlines-(textSize.cy)-NIBLSCALE(2) , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITENOREV, false);

  _tcscpy(Buffer, _T(""));
  if (GPSAltitudeOffset != 0) _stprintf(Buffer, _T("(GpsOffset %+.0f)"), GPSAltitudeOffset/1000*ALTITUDEMODIFY); // 100429 /1000
  #if TESTBENCH
  _tcscat(Buffer,_T(" TESTBENCH! "));
  #endif
  if (!LoggerGActive()) _tcscat(Buffer,_T(" (No GRecord)"));
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &textSize);
  LKWriteText(Surface, Buffer, centerx, bottomlines-(textSize.cy*2)-NIBLSCALE(2) , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITENOREV, false);
}

