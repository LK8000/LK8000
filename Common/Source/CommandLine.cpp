/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#if (WINDOWSPC>0)
// This is a quick solution to tell profiles not to override a command line choice, for v5
bool CommandResolution=false;
#endif

#ifndef UNDER_CE
void LK8000GetOpts()
#else
void LK8000GetOpts()
#endif
{

#if (WINDOWSPC>0) 
  SCREENWIDTH=800;
  SCREENHEIGHT=480;

  #if defined(SCREENWIDTH_)
  SCREENWIDTH=SCREENWIDTH_;
  #endif
  #if defined(SCREENHEIGHT_)
  SCREENHEIGHT=SCREENHEIGHT_;
  #endif
  CommandResolution=false;

  TCHAR *MyCommandLine = GetCommandLine();

  if (MyCommandLine != NULL){
    TCHAR *pC, *pCe;

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

			int x=_ttoi(stx);
			int y=_ttoi(sty);
			if (x>100 && x<3000 && y>100 && y<3000) {
				SCREENWIDTH=x;
				SCREENHEIGHT=y;
                                CommandResolution=true;
			}
		}
	}

    pC = _tcsstr(MyCommandLine, TEXT("-640x480"));
    if (pC != NULL){
      SCREENWIDTH=640;
      SCREENHEIGHT=480;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-800x480"));
    if (pC != NULL){
      SCREENWIDTH=800;
      SCREENHEIGHT=480;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-720x408"));
    if (pC != NULL){
      SCREENWIDTH=720;
      SCREENHEIGHT=408;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-800x600"));
    if (pC != NULL){
      SCREENWIDTH=800;
      SCREENHEIGHT=600;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-896x672"));
    if (pC != NULL){
      SCREENWIDTH=896;
      SCREENHEIGHT=672;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-854x358"));
    if (pC != NULL){
      SCREENWIDTH=854;
      SCREENHEIGHT=358;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-big"));
    if (pC != NULL){
      SCREENWIDTH=896;
      SCREENHEIGHT=672;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-400x240"));
    if (pC != NULL){
      SCREENWIDTH=400;
      SCREENHEIGHT=240;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x272"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=272;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x234"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=234;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x800"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=800;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-portrait"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=640;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x640"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=640;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-small"));
    if (pC != NULL){
      SCREENWIDTH/= 2;
      SCREENHEIGHT/= 2;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-320x240"));
    if (pC != NULL){
      SCREENWIDTH=320;
      SCREENHEIGHT=240;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-320x234"));
    if (pC != NULL){
      SCREENWIDTH=320;
      SCREENHEIGHT=234;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-240x320"));
    if (pC != NULL){
      SCREENWIDTH=240;
      SCREENHEIGHT=320;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-234x320"));
    if (pC != NULL){
      SCREENWIDTH=234;
      SCREENHEIGHT=320;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-240x400"));
    if (pC != NULL){
      SCREENWIDTH=240;
      SCREENHEIGHT=400;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-272x480"));
    if (pC != NULL){
      SCREENWIDTH=272;
      SCREENHEIGHT=480;
      CommandResolution=true;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-testmode"));
    if (pC != NULL){
      SCREENWIDTH=1018;
      SCREENHEIGHT=564;
      CommandResolution=true;
    }

  }
#else
  return;
#endif

}

