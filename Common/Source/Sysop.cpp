/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"

bool SysOpMode=false;

//
// System Operator commands syntax: OPC{args} 
// Notice, no spaces because textentry keyboard does not allow them.
// Return: true=close immediately dialogs  false=nothing to be done
//
// DPIn            force DPI in GetScreenDensity(); 0=use real function
// DTQn            change quantization to 0<=n<=16 (0=reset quantization to defaults)
// LCDnn           force screen size in inches . Value is *10:  50=5.0" default is 50
// RESnnnnXnnnn    change screen resolution to anything between 0200 and 4000 (padding 0 mandatory)
// SYSOP           activate sysop mode (also from command line -sysop)
// TST             test service for quick custom action, ex. change a bool, or anything with no arguments
// VARnn           change variable value, custom for developer 
// 
//
//

bool Sysop(TCHAR *command) {

   size_t len = _tcslen(command);
   if (len<3) {
      StartupStore(_T(". SYSOP: null or too short command: <%s>%s"),command,NEWLINE);
      return false;
   }

   //
   // SYSOP mode activation
   //
   if (_tcscmp(command,_T("SYSOP"))==0) {
      DoStatusMessage(_T("SYSOP MODE ACTIVATED!"));
      SysOpMode=true;
      StartupStore(_T(". SYSOP MODE ACTIVATED%s"),NEWLINE);
      return true;
   }

   TCHAR opc[4];
   opc[0]=command[0];
   opc[1]=command[1];
   opc[2]=command[2];
   opc[3]='\0';
   StartupStore(_T(". SYSOP: command: <%s>%s"),command,NEWLINE);

   //
   // Example DPI300  // force 300 DPI in GetScreenDensity()
   //
   if (_tcscmp(opc,_T("DPI"))==0) {
      if (len<4) {
         StartupStore(_T(". SYSOP invalid argument%s"),NEWLINE);
         return false;
      }
      unsigned int dp=(unsigned short)_tcstol(&command[3],NULL,10);
      if (dp<=2000) {
         extern unsigned short DpiSize;
         DpiSize=dp;
         StartupStore(_T(". SYSOP DPI=%d%s"),DpiSize,NEWLINE);
      } else
         StartupStore(_T(". SYSOP invalid argument%s"),NEWLINE);
      return false;
   }

   //
   // Example DTQ6
   //
   if (_tcscmp(opc,_T("DTQ"))==0) {
      if (len<4) {
         StartupStore(_T(". SYSOP invalid argument%s"),NEWLINE);
         return false;
      }
      unsigned int q=(unsigned short)_tcstol(&command[3],NULL,10);
      if (q<=16) {
         extern unsigned int CommandQuantization;
         CommandQuantization=q;
         StartupStore(_T(". SYSOP QUANTIZATION=%d%s"),CommandQuantization,NEWLINE);
      } else
         StartupStore(_T(". SYSOP invalid argument%s"),NEWLINE);
      return false;
   }

   //
   // Example LCD50   5.0" lcd 
   //
   if (_tcscmp(opc,_T("LCD"))==0) {
      if (len<5) {
         StartupStore(_T(". SYSOP invalid argument%s"),NEWLINE);
         return false;
      }
      unsigned int l=(unsigned short)_tcstol(&command[3],NULL,10);
      if (l<20 || l>240) {
         StartupStore(_T(". SYSOP invalid argument%s"),NEWLINE);
      } else {
         extern unsigned short LcdSize;
         LcdSize=(unsigned short) l;
         StartupStore(_T(". SYSOP change LcdSize to %d%s"),LcdSize,NEWLINE);
      }
      return false;
   }


   //
   // Example RES1024x0768 
   // 
   if (_tcscmp(opc,_T("RES"))==0) {
      if (len==12 && command[7]=='X') {
         TCHAR tst[5];
         tst[0]=command[3]; tst[1]=command[4]; tst[2]=command[5]; tst[3]=command[6];
         unsigned short esx=(unsigned short)_tcstol(tst,NULL,10);
         tst[0]=command[8]; tst[1]=command[9]; tst[2]=command[10]; tst[3]=command[11];
         unsigned short esy=(unsigned short)_tcstol(tst,NULL,10);

         if (esx>200 && esx<4000 && esy>200 && esy<4000) {
            StartupStore(_T(". SYSOP change screen resolution to %d x %d%s"),esx,esy,NEWLINE);
            RECT w=WindowResize(esx,esy);
            main_window->Resize(w.right-w.left, w.bottom-w.top);
            return true; // exit dialogs!
         } else
            StartupStore(_T(". SYSOP invalid change screen resolution %d x %d%s"),esx,esy,NEWLINE);
      } else {
         StartupStore(_T(". SYSOP invalid argument%s"),NEWLINE);
      }
   }

   //
   // TST ready for quick change on local repository
   //
   if (_tcscmp(command,_T("TST"))==0) {
      // Anything for a quick action
      DrawBottom=!DrawBottom;
      StartupStore(_T(". SYSOP TST EXECUTED%s"),NEWLINE);
      return true;
   }

   //
   // VARn ready for quick change on local repository
   //
   if (_tcscmp(opc,_T("VAR"))==0) {
      if (len<4) {
         StartupStore(_T(". SYSOP invalid argument%s"),NEWLINE);
         return false;
      }
      unsigned int n=(unsigned int)_tcstol(&command[3],NULL,10);
      if (n>0 && n<9999) {
         StartupStore(_T(". SYSOP VAR=%d%s"),n,NEWLINE);
         ScreenGeometry=n;
      } else
         StartupStore(_T(". SYSOP invalid argument%s"),NEWLINE);
      return false;
   }

   return false;
}
