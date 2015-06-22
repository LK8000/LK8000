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

  SIZE textSize;
  TCHAR Buffer[LKSIZEBUFFERLARGE];

  int x=rc.left+NIBLSCALE(4);
  int y=rc.top+NIBLSCALE(2);

  static double freeram=CheckFreeRam()/1000000.0;

  Surface.SelectObject(MapWindowBoldFont);


  #ifndef LKCOMPETITION
  _stprintf(Buffer,TEXT("%s v%s.%s %s"),_T(LKFORK),_T(LKVERSION),_T(LKRELEASE),_T(__DATE__));
  #else
  _stprintf(Buffer,TEXT("%sC v%s.%s %s"),_T(LKFORK),_T(LKVERSION),_T(LKRELEASE),_T(__DATE__));
  #endif
  Surface.GetTextSize(Buffer, _tcslen(Buffer), &textSize);
  y+=(textSize.cy)/2;
  LKWriteText(Surface, Buffer, x, y , 0, WTMODE_NORMAL, WTALIGN_LEFT,RGB_WHITENOREV, false);

  #ifdef LKCOMPETITION
  y+=(textSize.cy);
  _stprintf(Buffer,_T("COMPETITION VERSION"));
  LKWriteText(Surface, Buffer, x, y , 0, WTMODE_NORMAL, WTALIGN_LEFT,RGB_WHITENOREV, false);
  #endif

  _tcscpy(Buffer,_T(""));
  if (SIMMODE) _stprintf(Buffer,_T("(simul)"));
  #if TESTBENCH
  _tcscat(Buffer,_T(" (test)"));
  #endif
  #ifndef NDEBUG
  _tcscat(Buffer,_T(" (debug)"));
  #endif
  #if BUGSTOP
  _tcscat(Buffer,_T(" (bstop)"));
  #endif
  #if USELKASSERT
  _tcscat(Buffer,_T(" (assert)"));
  #endif
  if (_tcslen(Buffer)>0) {
      y+=(textSize.cy);
      LKWriteText(Surface, Buffer, x, y , 0, WTMODE_NORMAL, WTALIGN_LEFT,RGB_WHITENOREV, false);
  }

  y+=(textSize.cy)/2; // spacing

  _stprintf(Buffer, _T("Waypoints loaded: %u"), (unsigned int)(WayPointList.size()-NUMRESWP));
  y+=(textSize.cy);
  LKWriteText(Surface, Buffer, x, y , 0, WTMODE_NORMAL, WTALIGN_LEFT,RGB_WHITENOREV, false);


  _stprintf(Buffer, _T("Free RAM: %0.1fM"),freeram);
  y+=(textSize.cy);
  LKWriteText(Surface, Buffer, x, y , 0, WTMODE_NORMAL, WTALIGN_LEFT,RGB_WHITENOREV, false);

  y+=(textSize.cy)/2; // spacing

  if (GPSAltitudeOffset != 0) {
      _stprintf(Buffer, _T("Reminder: HGPS offset: %+.0f)"), GPSAltitudeOffset/1000*ALTITUDEMODIFY); 
      y+=(textSize.cy);
      LKWriteText(Surface, Buffer, x, y , 0, WTMODE_NORMAL, WTALIGN_LEFT,RGB_WHITENOREV, false);
  }

  _stprintf(Buffer,TEXT("Click to continue"));
  y= rc.bottom-BottomSize-textSize.cy-NIBLSCALE(2);
  LKWriteText(Surface, Buffer, x, y , 0, WTMODE_NORMAL, WTALIGN_LEFT,RGB_WHITENOREV, false);


}

