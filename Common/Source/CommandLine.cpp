/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils.cpp,v 8.17 2010/12/19 16:42:53 root Exp root $
*/

#include "StdAfx.h"

#ifndef __MINGW32__
#if defined(CECORE)
#include "winbase.h"
#endif
#if (WINDOWSPC<1)
#include "projects.h"
#endif
#else
#include "wcecompat/ts_string.h"
#endif

#include "options.h"
#include "Defines.h"
#include "externs.h"
#include "lk8000.h"

#ifdef __MINGW32__
#ifndef max
#define max(x, y)   (x > y ? x : y)
#define min(x, y)   (x < y ? x : y)
#endif
#endif

void LK8000GetOpts(LPTSTR CommandLine) {
  (void)CommandLine;

  TCHAR buffer[MAX_PATH];
#if (!defined(WINDOWSPC) || (WINDOWSPC <=0) )
  LocalPath(buffer,TEXT(LKD_CONF));
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,_T(XCSPROFILE)); // 091101

#else
  SHGetSpecialFolderPath(hWndMainWindow, buffer, CSIDL_PERSONAL, false);
  _tcscat(buffer,TEXT("\\"));
  _tcscat(buffer,TEXT(XCSDATADIR));
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,TEXT(LKD_CONF)); // 091101
  _tcscat(buffer,_T("\\"));
  _tcscat(buffer,_T(XCSPROFILE)); // 091101
#endif
  _tcscpy(defaultProfileFile,buffer);
  _tcscpy(startProfileFile, defaultProfileFile);

#if (WINDOWSPC>0) 
  SCREENWIDTH=800;
  SCREENHEIGHT=480;

#if defined(SCREENWIDTH_)
  SCREENWIDTH=SCREENWIDTH_;
#endif
#if defined(SCREENHEIGHT_)
  SCREENHEIGHT=SCREENHEIGHT_;
#endif

#else
  return; // don't do anything for PDA platforms
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

        _tcsncpy(startProfileFile, pC, pCe-pC);
        startProfileFile[pCe-pC] = '\0';
      }
    }
#if (WINDOWSPC>0) 
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

#endif
  }
}

