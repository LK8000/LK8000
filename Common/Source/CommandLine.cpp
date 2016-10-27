/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#if !defined(UNDER_CE) || defined(__linux__)

// This is a quick solution to tell profiles not to override a command line choice, for v5
bool CommandResolution=false;

void LK8000GetOpts(const TCHAR *MyCommandLine) {

  CommandResolution=false;

  if (MyCommandLine != NULL){
    const TCHAR *pC, *pCe;

    pC = _tcsstr(MyCommandLine, TEXT("-profile="));
    if (pC != NULL){
      pC += strlen("-profile=");
      if (*pC == '"'){
        pC++;
        pCe = pC;
        while (*pCe != '"' && *pCe != '\0') pCe++;
      } else{
        pCe = pC;
        while (*pCe != ' ' && *pCe != '\0') pCe++;
      }
      if (pCe != NULL && pCe-1 > pC){

        LK_tcsncpy(startProfileFile, pC, pCe-pC);
      }
    }

	//
	// custom resolution setup
	//
	pC = _tcsstr(MyCommandLine, TEXT("-x="));
	if (pC != NULL) {
		TCHAR stx[10];
		_tcscpy(stx,_T(""));
		pC += strlen("-x=");
		if (*pC == '"'){
			pC++;
			pCe = pC;
			while (*pCe != '"' && *pCe != '\0') pCe++;
		} else{
			pCe = pC;
			while (*pCe != ' ' && *pCe != '\0') pCe++;
		}
		if (pCe != NULL && pCe-1 > pC) {
			LK_tcsncpy(stx, pC, pCe-pC);
		}

		pC = _tcsstr(MyCommandLine, TEXT("-y="));
		if (pC != NULL) {
			TCHAR sty[10];
			_tcscpy(sty,_T(""));
			pC += strlen("-y=");
			if (*pC == '"') {
				pC++;
				pCe = pC;
				while (*pCe != '"' && *pCe != '\0') pCe++;
			} else {
				pCe = pC;
				while (*pCe != ' ' && *pCe != '\0') pCe++;
			}
			if (pCe != NULL && pCe-1 > pC) {
				LK_tcsncpy(sty, pC, pCe-pC);
			}

			int x=_tcstol(stx, nullptr, 10);
			int y=_tcstol(sty, nullptr, 10);
			if (x>=240 && x<=4000 && y>=240 && y<=4000) {
				ScreenSizeX=x;
				ScreenSizeY=y;
        CommandResolution=true;
			}
		}
	}

    pC = _tcsstr(MyCommandLine, TEXT("-640x480"));
    if (pC != NULL){
      ScreenSizeX=640;
      ScreenSizeY=480;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-800x480"));
    if (pC != NULL){
      ScreenSizeX=800;
      ScreenSizeY=480;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-720x408"));
    if (pC != NULL){
      ScreenSizeX=720;
      ScreenSizeY=408;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-800x600"));
    if (pC != NULL){
      ScreenSizeX=800;
      ScreenSizeY=600;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-896x672"));
    if (pC != NULL){
      ScreenSizeX=896;
      ScreenSizeY=672;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-854x358"));
    if (pC != NULL){
      ScreenSizeX=854;
      ScreenSizeY=358;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-big"));
    if (pC != NULL){
      ScreenSizeX=896;
      ScreenSizeY=672;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-400x240"));
    if (pC != NULL){
      ScreenSizeX=400;
      ScreenSizeY=240;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x272"));
    if (pC != NULL){
      ScreenSizeX=480;
      ScreenSizeY=272;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x234"));
    if (pC != NULL){
      ScreenSizeX=480;
      ScreenSizeY=234;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x800"));
    if (pC != NULL){
      ScreenSizeX=480;
      ScreenSizeY=800;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-portrait"));
    if (pC != NULL){
      ScreenSizeX=480;
      ScreenSizeY=640;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x640"));
    if (pC != NULL){
      ScreenSizeX=480;
      ScreenSizeY=640;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-small"));
    if (pC != NULL){
      ScreenSizeX/= 2;
      ScreenSizeY/= 2;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-320x240"));
    if (pC != NULL){
      ScreenSizeX=320;
      ScreenSizeY=240;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-320x234"));
    if (pC != NULL){
      ScreenSizeX=320;
      ScreenSizeY=234;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-240x320"));
    if (pC != NULL){
      ScreenSizeX=240;
      ScreenSizeY=320;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-234x320"));
    if (pC != NULL){
      ScreenSizeX=234;
      ScreenSizeY=320;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-240x400"));
    if (pC != NULL){
      ScreenSizeX=240;
      ScreenSizeY=400;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-272x480"));
    if (pC != NULL){
      ScreenSizeX=272;
      ScreenSizeY=480;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-testmode"));
    if (pC != NULL){
      ScreenSizeX=1018;
      ScreenSizeY=564;
      CommandResolution=true;
    }

  }

}

#endif