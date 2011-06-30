/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#if !defined(__DLGTOOLS_H)
#define __DLGTOOLS_H

#include "WindowControls.h"

#define DeclareCallBackEntry(x)        {TEXT(#x), (void *)x}
int DLGSCALE(int x);

typedef struct{
  const TCHAR *Name;
  void *Ptr;
}CallBackTableEntry_t;

WndForm *dlgLoadFromXML(CallBackTableEntry_t *LookUpTable, const char *FileName, 
			HWND Parent, const TCHAR *resource=NULL);

int
WINAPI
MessageBoxX(
    HWND hWnd ,
    LPCWSTR lpText,
    LPCWSTR lpCaption,
    UINT uType);

#endif
