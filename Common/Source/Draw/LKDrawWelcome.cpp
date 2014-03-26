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


void MapWindow::DrawWelcome8000(HDC hdc, RECT rc) {

  SIZE textSize, headerSize;
  TCHAR Buffer[LKSIZEBUFFERLARGE];

  short bottomlines;
  short middlex=(rc.right-rc.left)/2;
  //short left=rc.left+NIBLSCALE(5);
  short contenttop=rc.top+NIBLSCALE(50);
#if TESTBENCH
  static double freeram=CheckFreeRam()/1000000.0;
#endif

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
  
  SelectObject(hdc, LK8BigFont);
  _stprintf(Buffer,TEXT("LK8000"));
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &headerSize);
  LKWriteText(hdc, Buffer, middlex, (headerSize.cy/2)+NIBLSCALE(2) , 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_WHITENOREV, false);

  _stprintf(Buffer,gettext(TEXT("_@M904_"))); // Tactical Flight Computer
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &textSize);
  SelectObject(hdc, LK8MediumFont);
  LKWriteText(hdc, Buffer, middlex, (headerSize.cy/2)+(textSize.cy/2)+NIBLSCALE(4)+1 , 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_WHITENOREV, false);


  //SelectObject(hdc, LK8InfoBigFont);
  SelectObject(hdc, LK8TitleFont);
#ifndef LKCOMPETITION
  _stprintf(Buffer,TEXT("%s v%s.%s"),_T(LKFORK),_T(LKVERSION),_T(LKRELEASE));
#else
  _stprintf(Buffer,TEXT("%sC v%s.%s COMPETITION"),_T(LKFORK),_T(LKVERSION),_T(LKRELEASE));
#endif
  if (SIMMODE) _tcscat(Buffer,_T(" (Simulator)"));
  LKWriteText(hdc, Buffer, middlex, contenttop+(textSize.cy*1) , 0, WTMODE_OUTLINED, WTALIGN_CENTER,RGB_AMBERNOREV, false);


  _stprintf(Buffer,gettext(TEXT("_@M874_"))); // Click on center screen to begin
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &textSize);
  LKWriteText(hdc, Buffer, middlex, ((rc.bottom-rc.top)-textSize.cy)/2 , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITENOREV, false);


  SelectObject(hdc, LK8UnitFont);
  if (ScreenSize==0) {
	_stprintf(Buffer,TEXT("Screen %ldx%ld experimental"),rc.right,rc.bottom );
	GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &textSize);
	bottomlines=rc.bottom-BottomSize-(textSize.cy*3);
	LKWriteText(hdc, Buffer, middlex, bottomlines , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITENOREV, false);
  } else {
	_stprintf(Buffer,TEXT("%s"), LK8000_Version);
	GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &textSize);
	bottomlines=rc.bottom-BottomSize-(textSize.cy*3);
	LKWriteText(hdc, Buffer, middlex, bottomlines , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITE, false);
	_stprintf(Buffer,TEXT("HTTP://WWW.LK8000.IT  email:info@lk8000.it"));
	LKWriteText(hdc, Buffer, middlex, bottomlines+textSize.cy , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITENOREV, false);
 }

  SelectObject(hdc, LK8InfoSmallFont);

#if TESTBENCH
  _stprintf(Buffer, _T("%d WPs, %0.1fM free"),NumberOfWayPoints-NUMRESWP,freeram);
#else
  _tcscpy(Buffer,_T(""));
#endif
  if (PGNumberOfGates>0) _tcscat(Buffer,_T(" (+Tsk Gates)"));
#ifndef NDEBUG
  _tcscat(Buffer,_T(" (+debug)"));
#endif
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &textSize);
  LKWriteText(hdc, Buffer, middlex, bottomlines-(textSize.cy)-NIBLSCALE(2) , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITENOREV, false);

  _tcscpy(Buffer, _T(""));
  if (GPSAltitudeOffset != 0) _stprintf(Buffer, _T("(GpsOffset %+.0f)"), GPSAltitudeOffset/1000*ALTITUDEMODIFY); // 100429 /1000
  #if TESTBENCH
  _tcscat(Buffer,_T(" TESTBENCH! "));
  #endif
  if (!LoggerGActive()) _tcscat(Buffer,_T(" (No GRecord)"));
  GetTextExtentPoint(hdc, Buffer, _tcslen(Buffer), &textSize);
  LKWriteText(hdc, Buffer, middlex, bottomlines-(textSize.cy*2)-NIBLSCALE(2) , 0, WTMODE_NORMAL, WTALIGN_CENTER, RGB_WHITENOREV, false);

  if (WarningHomeDir) {
	TCHAR nopath[MAX_PATH];
	LocalPath(nopath,_T(""));
	// LKTOKEN _@M1209_ "CHECK INSTALLATION!"
	MessageBoxX(hWndMapWindow, nopath, gettext(TEXT("_@M1209_")), MB_OK|MB_ICONEXCLAMATION);
	WarningHomeDir=false;
  }
#if ( WINDOWSPC==0 )
  static bool checktickcountbug=true; // 100510
  if (checktickcountbug) {
	DWORD counts=GetTickCount();
	if (counts >(unsigned)2073600000l) {
	// LKTOKEN  _@M527_ = "Please exit LK8000 and reset your device.\n" 
		MessageBoxX(hWndMapWindow, gettext(TEXT("_@M527_")),
                TEXT("Device need reset!"),
                MB_OK|MB_ICONEXCLAMATION);
	}
	checktickcountbug=false;
  }
#endif
  static bool checksafetyaltitude=true; // 100709
  if (!ISPARAGLIDER && !ISCAR) // 100925
  if (checksafetyaltitude) {
	if (SAFETYALTITUDEARRIVAL<500) { // SAFETY is *10, so we check <50 really
	// LKTOKEN  _@M155_ = "CHECK safety arrival altitude\n" 
		MessageBoxX(hWndMapWindow, gettext(TEXT("_@M155_")),
                TEXT("Warning!"),
                MB_OK|MB_ICONEXCLAMATION);
	}
	checksafetyaltitude=false;
  }
  return;
}

