/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include <stdio.h>
#include "Modeltype.h"
#if defined(PNA) && defined(UNDER_CE)
#include "Devices/LKHolux.h"
#include "Devices/LKRoyaltek3200.h"

/*
 * SetBacklight for PNA devices. There is no standard way of managing backlight on CE,
 * and every device may have different value names and settings. Microsoft did not set
 * a standard and thus we need a custom solution for each device.
 * But the approach is always the same: change a value and call an event.
 */
bool SetBacklight() // VENTA4
{
  HKEY    hKey;
  DWORD   Disp=0;
  HRESULT hRes;
  HANDLE BLEvent;

  if (EnableAutoBacklight == false ) return false;


  switch (GlobalModelType)
  {
	case MODELTYPE_PNA_HP31X:

		hRes = RegOpenKeyEx(HKEY_CURRENT_USER, _T("ControlPanel\\Backlight"), 0,  0, &hKey);
		if (hRes != ERROR_SUCCESS) return false;

		Disp=20; // max backlight
		// currently we ignore hres, if registry entries are spoiled out user is already in deep troubles
		hRes = RegSetValueEx(hKey, _T("BackLightCurrentACLevel"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("BackLightCurrentBatteryLevel"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("TotalLevels"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=0;
		hRes = RegSetValueEx(hKey, _T("UseExt"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		RegDeleteValue(hKey,_T("ACTimeout"));
		RegCloseKey(hKey);
		BLEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("BacklightChangeEvent"));
		if ( SetEvent(BLEvent) == 0)
			return false;
		else
			CloseHandle(BLEvent);

		break;

	case MODELTYPE_PNA_ROYALTEK3200:

		hRes = RegOpenKeyEx(HKEY_CURRENT_USER, _T("ControlPanel\\Backlight"), 0,  0, &hKey);
		if (hRes != ERROR_SUCCESS) return false;

		// currently we ignore hres, if registry entries are spoiled out user is already in deep troubles

		Disp=0; // disable timeouts
		hRes = RegSetValueEx(hKey, _T("ACTimeout"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("BatteryTimeout"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("AutoChageBrightness"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));

		Disp=60; // max backlight
		hRes = RegSetValueEx(hKey, _T("BattBrightness"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		hRes = RegSetValueEx(hKey, _T("ACBrightness"),0,REG_DWORD, (LPBYTE)&Disp, sizeof(DWORD));
		Disp=0;
		RegCloseKey(hKey);
		BLEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("BacklightChangeEvent"));
		if ( SetEvent(BLEvent) == 0)
			return false;
		else
			CloseHandle(BLEvent);

		break;

	case MODELTYPE_PNA_FUNTREK:

		GM130MaxBacklight();

		break;
	default:
		return false;
		break;
  }

  #if TESTBENCH
  StartupStore(_T("...... Backlight set ok!\n"));
  #endif
  return true;

}

#endif
