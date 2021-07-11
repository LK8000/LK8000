/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

#if !defined(UNDER_CE) || defined(__linux__) && !defined(ANDROID)

// This is a quick solution to tell profiles not to override a command line choice, for v5
bool CommandResolution=false;

extern unsigned short LcdSize, DpiSize;
extern unsigned short ReferenceDpi;
extern unsigned int CommandQuantization;

//
//  true,  continue normally
//  false, no startup
// 

bool LK8000GetOpts(const TCHAR *MyCommandLine) {

  CommandResolution=false;

  if (MyCommandLine == NULL) return true;

  const TCHAR *pC, *pCe;
  TCHAR mytext1[10], mytext2[10];
  bool validx=false, validy=false;

  pC = _tcsstr(MyCommandLine, TEXT("-sysop"));
  if (pC != NULL){
     extern bool SysOpMode;
     SysOpMode=true;
  }

  pC = _tcsstr(MyCommandLine, TEXT("-help"));
  if (pC != NULL){

     fprintf(stderr,"\n\
 **************** LK8000 command line help ******************\n\
 -help\n\
 -profile=filename\n\
          load filename as StartProfileFile\n\
 -x=n -y=n\n\
          use specified screen resolution, example -x=480 -y=272\n\
 -lcdsize=n\n\
          assume monitor size is n inches. The value must be multiplied by 10\n\
          and must be integer. Example for 4.5 inches, use -lcdsize=45 \n\
 -dpi=n\n\
          assume screen density has n dpi , example -dpi=144\n\
          This value ovverrides -lcdsize\n\
 -refdpi=n\n\
          set reference DPI, default to 80.\n\
 -quant=n\n\
          force terrain quantization=n\n\
 -sysop\n\
          start with sysop mode active\n\
\n");

  return false; 

  }


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
     if (pCe != NULL && pCe-1 > pC) {
        LK_tcsncpy(startProfileFile, pC, pCe-pC);
     }
  }

  //
  // custom resolution setup
  //
  pC = _tcsstr(MyCommandLine, TEXT("-x="));
  if (pC != NULL) {
     _tcscpy(mytext1,_T(""));
      pC += strlen("-x=");
     if (*pC == '"'){
        pC++;
        pCe = pC;
        while (*pCe != '"' && *pCe != '\0') pCe++;
     } else{
        pCe = pC;
        while (*pCe != ' ' && *pCe != '\0') pCe++;
     }
     if (pCe != NULL && pCe > pC) {
        LK_tcsncpy(mytext1, pC, pCe-pC);
        validx=true;
     }
  }

  pC = _tcsstr(MyCommandLine, TEXT("-y="));
  if (pC != NULL) {
     _tcscpy(mytext2,_T(""));
      pC += strlen("-y=");
     if (*pC == '"') {
        pC++;
        pCe = pC;
        while (*pCe != '"' && *pCe != '\0') pCe++;
     } else {
        pCe = pC;
        while (*pCe != ' ' && *pCe != '\0') pCe++;
     }
     if (pCe != NULL && pCe > pC) {
        LK_tcsncpy(mytext2, pC, pCe-pC);
        validy=true;
     }
  }

  if (validx && validy) {
     int x=_tcstol(mytext1, nullptr, 10);
     int y=_tcstol(mytext2, nullptr, 10);
     if (x>=100 && x<=5000 && y>=100 && y<=5000) {
        ScreenSizeX=x;
        ScreenSizeY=y;
        StartupStore(_T(". CommandLine ScreenSize x=%d y=%d%s"),x,y,NEWLINE);
        CommandResolution=true;
     }
  }

  pC = _tcsstr(MyCommandLine, TEXT("-lcdsize="));
  if (pC != NULL){
     _tcscpy(mytext1,_T(""));
     pC += strlen("-lcdsize=");
     if (*pC == '"'){
        pC++;
        pCe = pC;
        while (*pCe != '"' && *pCe != '\0') pCe++;
     } else{
        pCe = pC;
        while (*pCe != ' ' && *pCe != '\0') pCe++;
     }
     if (pCe != NULL && pCe > pC) {
        LK_tcsncpy(mytext1, pC, pCe-pC);
        int s=_tcstol(mytext1, nullptr, 10);
        if (s<20 || s >240) {
           StartupStore(_T(". CommandLine monitor size=%d is out of range%s"),s,NEWLINE);
        } else {
           StartupStore(_T(". CommandLine monitor size=%d inches%s"),s,NEWLINE);
           LcdSize=s;
        }
     }
  }

  pC = _tcsstr(MyCommandLine, TEXT("-dpi="));
  if (pC != NULL){
     _tcscpy(mytext1,_T(""));
     pC += strlen("-dpi=");
     if (*pC == '"'){
        pC++;
        pCe = pC;
        while (*pCe != '"' && *pCe != '\0') pCe++;
     } else{
        pCe = pC;
        while (*pCe != ' ' && *pCe != '\0') pCe++;
     }
     if (pCe != NULL && pCe > pC) {
        LK_tcsncpy(mytext1, pC, pCe-pC);
        int s=_tcstol(mytext1, nullptr, 10);
        if (s<40 || s >2000) {
           StartupStore(_T(". CommandLine dpi=%d is out of range%s"),s,NEWLINE);
        } else {
           StartupStore(_T(". CommandLine dpi=%d inches%s"),s,NEWLINE);
           DpiSize=s;
        }
     }
  }

  pC = _tcsstr(MyCommandLine, TEXT("-refdpi="));
  if (pC != NULL){
     _tcscpy(mytext1,_T(""));
     pC += strlen("-refdpi=");
     if (*pC == '"'){
        pC++;
        pCe = pC;
        while (*pCe != '"' && *pCe != '\0') pCe++;
     } else{
        pCe = pC;
        while (*pCe != ' ' && *pCe != '\0') pCe++;
     }
     if (pCe != NULL && pCe > pC) {
        LK_tcsncpy(mytext1, pC, pCe-pC);
        int s=_tcstol(mytext1, nullptr, 10);
        if (s<10 || s >500) {
           StartupStore(_T(". CommandLine refdpi=%d is out of range%s"),s,NEWLINE);
        } else {
           StartupStore(_T(". CommandLine refdpi=%d inches%s"),s,NEWLINE);
           ReferenceDpi=s;
        }
     }
  }

  pC = _tcsstr(MyCommandLine, TEXT("-quant="));
  if (pC != NULL){
     _tcscpy(mytext1,_T(""));
     pC += strlen("-quant=");
     if (*pC == '"'){
        pC++;
        pCe = pC;
        while (*pCe != '"' && *pCe != '\0') pCe++;
     } else{
        pCe = pC;
        while (*pCe != ' ' && *pCe != '\0') pCe++;
     }
     if (pCe != NULL && pCe > pC) {
        LK_tcsncpy(mytext1, pC, pCe-pC);
        int s=_tcstol(mytext1, nullptr, 10);
        if (s<1 || s >16) {
           StartupStore(_T(". CommandLine quant=%d is out of range 1-16%s"),s,NEWLINE);
        } else {
           StartupStore(_T(". CommandLine terrain quantization=%d %s"),s,NEWLINE);
           CommandQuantization=s;
        }
     }
  }

  return true;
}

#endif
