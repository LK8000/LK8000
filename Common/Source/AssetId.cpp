/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
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
#include "uniqueid.h"


void ReadAssetNumber(void)
{
  TCHAR val[MAX_PATH];

  val[0]= _T('\0');

  memset(strAssetNumber, 0, MAX_LOADSTRING*sizeof(TCHAR));
  // JMW clear this first just to be safe.

  StartupStore(TEXT(". Asset ID: "));

#if (WINDOWSPC>0)
	strAssetNumber[0]= _T('L');
	strAssetNumber[1]= _T('K');
	strAssetNumber[2]= _T('8');
	#if TESTBENCH
	StartupStore(strAssetNumber);
	StartupStore(TEXT(" (PC)%s"),NEWLINE);
	#endif
	return;
#endif

  GetRegistryString(szRegistryLoggerID, val, 100);
  int ifound=0;
  int len = _tcslen(val);
  for (int i=0; i< len; i++) {
    if (((val[i] >= _T('A'))&&(val[i] <= _T('Z')))
        ||((val[i] >= _T('0'))&&(val[i] <= _T('9')))) {
      strAssetNumber[ifound]= val[i];
      ifound++;
    }
    if (ifound>=3) {
// TODO metti a 0 l'ultimo byte per chiudere la stringa
      StartupStore(strAssetNumber);
      StartupStore(TEXT(" (reg)%s"),NEWLINE);
      return;
    }
  }

  if (ifound>0 && ifound<3) {
	if (ifound==1) strAssetNumber[1]= _T('A');
	strAssetNumber[2]= _T('A');
  }

  if(strAssetNumber[0] != '\0')
    {
      StartupStore(strAssetNumber);
      StartupStore(TEXT(" (?)%s"),NEWLINE);
      return;
    }

  ReadUUID();
  if(strAssetNumber[0] != '\0')
    {
      StartupStore(strAssetNumber);
      StartupStore(TEXT(" (uuid)%s"),NEWLINE);
      return;
    }
  
  strAssetNumber[0]= _T('A');
  strAssetNumber[1]= _T('A');
  strAssetNumber[2]= _T('A');

  StartupStore(strAssetNumber);
  StartupStore(TEXT(" (fallback)%s"),NEWLINE);
  
  return;
}


void ReadUUID(void) 
{
#if !(defined(__MINGW32__) && (WINDOWSPC>0)) && !defined(_MSC_VER)
  BOOL fRes;
  
#define GUIDBuffsize 100
  unsigned char GUIDbuffer[GUIDBuffsize];

  int eLast=0;
  int i;
  unsigned long uNumReturned=0;
  int iBuffSizeIn=0;
  unsigned long temp, Asset;


  GUID Guid;

  
  // approach followed: http://blogs.msdn.com/jehance/archive/2004/07/12/181116.aspx
  // 1) send 16 byte buffer - some older devices need this
  // 2) if buffer is wrong size, resize buffer accordingly and retry
  // 3) take first 16 bytes of buffer and process.  Buffer returned may be any size 
  // First try exactly 16 bytes, some older PDAs require exactly 16 byte buffer

      #ifdef HAVEEXCEPTIONS
    __try {
      #else
	  strAssetNumber[0]= '\0';
      #endif

	  iBuffSizeIn=sizeof(Guid);
	  memset(GUIDbuffer, 0, iBuffSizeIn);
	  fRes = KernelIoControl(IOCTL_HAL_GET_DEVICEID, 0, 0, GUIDbuffer, iBuffSizeIn, &uNumReturned);
	  if(fRes == FALSE)
	  { // try larger buffer
		  eLast = GetLastError();
		  if (ERROR_INSUFFICIENT_BUFFER != eLast)
		  {
			return;
		  }
		  else
		  { // wrong buffer
			iBuffSizeIn = uNumReturned;
			memset(GUIDbuffer, 0, iBuffSizeIn);
			fRes = KernelIoControl(IOCTL_HAL_GET_DEVICEID, 0, 0, GUIDbuffer, iBuffSizeIn, &uNumReturned);
  			eLast = GetLastError();
			if(FALSE == fRes)
				return;
		  }
	  }

	  // here assume we have data in GUIDbuffer of length uNumReturned
	  memcpy(&Guid,GUIDbuffer, sizeof(Guid));
  

	  temp = Guid.Data2; temp = temp << 16;
	  temp += Guid.Data3 ;

	  Asset = temp ^ Guid.Data1 ;

	  temp = 0;
	  for(i=0;i<4;i++)
		{
		  temp = temp << 8;
		  temp += Guid.Data4[i];
		}

	  Asset = Asset ^ temp;

	  temp = 0;
	  for(i=0;i<4;i++)
		{
		  temp = temp << 8;
		  temp += Guid.Data4[i+4];
		}

	  Asset = Asset ^ temp;

	  _stprintf(strAssetNumber,TEXT("%08X%08X"),Asset,Guid.Data1 );

#ifdef HAVEEXCEPTIONS
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
	  strAssetNumber[0]= '\0';
  }
#endif
#endif
  return;
}

