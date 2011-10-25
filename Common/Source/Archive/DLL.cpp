/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

  $Id$
*/

#include "externs.h"
#include "InputEvents.h"

#include <commctrl.h>
#include <aygshell.h>


// DLL Cache
typedef void (CALLBACK *DLLFUNC_INPUTEVENT)(TCHAR*);
typedef void (CALLBACK *DLLFUNC_SETHINST)(HMODULE);


#define MAX_DLL_CACHE 256

typedef struct {
  TCHAR *text;
  HINSTANCE hinstance;
} DLLCACHESTRUCT;

DLLCACHESTRUCT DLLCache[MAX_DLL_CACHE];
int DLLCache_Count = 0;



// DLLExecute
// Runs the plugin of the specified filename
void InputEvents::eventDLLExecute(const TCHAR *misc) {
  // LoadLibrary(TEXT("test.dll"));

  StartupStore(TEXT("... %s%s"), misc,NEWLINE);
	
  TCHAR data[MAX_PATH];
  TCHAR* dll_name;
  TCHAR* func_name;
  TCHAR* other;
  TCHAR* pdest;
	
  _tcscpy(data, misc);

  // dll_name (up to first space)
  pdest = _tcsstr(data, TEXT(" "));
  if (pdest == NULL) {
    return;
  }
  *pdest = _T('\0');
  dll_name = data;

  // func_name (after first space)
  func_name = pdest + 1;

  // other (after next space to end of string)
  pdest = _tcsstr(func_name, TEXT(" "));
  if (pdest != NULL) {
    *pdest = _T('\0');
    other = pdest + 1;
  } else {
    other = NULL;
  }

  HINSTANCE hinstLib;	// Library pointer
  DLLFUNC_INPUTEVENT lpfnDLLProc = NULL;	// Function pointer

  // Load library, find function, execute, unload library
  hinstLib = _loadDLL(dll_name);
  if (hinstLib != NULL) {
#if !(defined(__MINGW32__)&&(WINDOWSPC>0))
    lpfnDLLProc = (DLLFUNC_INPUTEVENT)GetProcAddress(hinstLib, func_name);
#endif
    if (lpfnDLLProc != NULL) {
      (*lpfnDLLProc)(other);
    }
  }
}

// Load a DLL (only once, keep a cache of the handle)
//	TODO code: FreeLibrary - it would be nice to call FreeLibrary 
//      before exit on each of these
HINSTANCE _loadDLL(TCHAR *name) {
  int i;
  for (i = 0; i < DLLCache_Count; i++) {
    if (_tcscmp(name, DLLCache[i].text) == 0)
      return DLLCache[i].hinstance;
  }
  if (DLLCache_Count < MAX_DLL_CACHE) {
    DLLCache[DLLCache_Count].hinstance = LoadLibrary(name);
    if (DLLCache[DLLCache_Count].hinstance) {
      DLLCache[DLLCache_Count].text = StringMallocParse(name);
      DLLCache_Count++;
      
      // First time setup... (should check version numbers etc...)
      DLLFUNC_SETHINST lpfnDLLProc = NULL;
#if !(defined(__MINGW32__)&&(WINDOWSPC>0))
      lpfnDLLProc = (DLLFUNC_SETHINST)
	GetProcAddress(DLLCache[DLLCache_Count - 1].hinstance, 
		       TEXT("XCSAPI_SetHInst"));
#endif
      if (lpfnDLLProc)
	lpfnDLLProc(GetModuleHandle(NULL));
      
      return DLLCache[DLLCache_Count - 1].hinstance;
    }
  }

  return NULL;
}

