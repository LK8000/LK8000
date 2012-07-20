/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

void LK8000GetOpts() {
#if (WINDOWSPC>0) 
  SCREENWIDTH=800;
  SCREENHEIGHT=480;

  #if defined(SCREENWIDTH_)
  SCREENWIDTH=SCREENWIDTH_;
  #endif
  #if defined(SCREENHEIGHT_)
  SCREENHEIGHT=SCREENHEIGHT_;
  #endif

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

    pC = _tcsstr(MyCommandLine, TEXT("-640x480"));
    if (pC != NULL){
      SCREENWIDTH=640;
      SCREENHEIGHT=480;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-800x480"));
    if (pC != NULL){
      SCREENWIDTH=800;
      SCREENHEIGHT=480;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-720x408"));
    if (pC != NULL){
      SCREENWIDTH=720;
      SCREENHEIGHT=408;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-800x600"));
    if (pC != NULL){
      SCREENWIDTH=800;
      SCREENHEIGHT=600;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-896x672"));
    if (pC != NULL){
      SCREENWIDTH=896;
      SCREENHEIGHT=672;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-854x358"));
    if (pC != NULL){
      SCREENWIDTH=854;
      SCREENHEIGHT=358;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-big"));
    if (pC != NULL){
      SCREENWIDTH=896;
      SCREENHEIGHT=672;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-400x240"));
    if (pC != NULL){
      SCREENWIDTH=400;
      SCREENHEIGHT=240;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x272"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=272;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x234"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=234;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x800"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=800;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-portrait"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=640;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-480x640"));
    if (pC != NULL){
      SCREENWIDTH=480;
      SCREENHEIGHT=640;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-small"));
    if (pC != NULL){
      SCREENWIDTH/= 2;
      SCREENHEIGHT/= 2;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-320x240"));
    if (pC != NULL){
      SCREENWIDTH=320;
      SCREENHEIGHT=240;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-320x234"));
    if (pC != NULL){
      SCREENWIDTH=320;
      SCREENHEIGHT=234;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-240x320"));
    if (pC != NULL){
      SCREENWIDTH=240;
      SCREENHEIGHT=320;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-234x320"));
    if (pC != NULL){
      SCREENWIDTH=234;
      SCREENHEIGHT=320;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-240x400"));
    if (pC != NULL){
      SCREENWIDTH=240;
      SCREENHEIGHT=400;
    }
    pC = _tcsstr(MyCommandLine, TEXT("-272x480"));
    if (pC != NULL){
      SCREENWIDTH=272;
      SCREENHEIGHT=480;
    }

  }
#else
  return;
#endif

}

