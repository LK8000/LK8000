#if defined(PNA) && defined(UNDER_CE)

#include "externs.h"
#include "lk8000.h"
#include <iterator>
#include "BatteryManager.h"


/*  **********************************************************
 *
 *  Holux FunTrek 130 hardware support
 *  by Paolo Ventafridda for LK8000 Project, (C) July 2011
 *  With kind permission and support by Eddy Young, Holux Inc.
 *
 *  **********************************************************/

//
// Baro sensor
//
typedef BOOL (WINAPI *pGM130_barOpenDevice)(void);
pGM130_barOpenDevice GM130_barOpenDevice;
typedef void (WINAPI *pGM130_barCloseDevice)(void);
pGM130_barCloseDevice GM130_barCloseDevice;
typedef int (WINAPI *pGM130_barGetAltitude)(void);
pGM130_barGetAltitude GM130_barGetAltitude;
typedef float (WINAPI *pGM130_barGetPressure)(void);
pGM130_barGetPressure GM130_barGetPressure;
#if 0 // unused
typedef BOOL (WINAPI *pGM130_barCalibrate)(int CalValue);
pGM130_barCalibrate GM130_barCalibrate;
#endif

//
// Power manager
//
typedef BOOL (WINAPI *pGM130_pwrOpenDevice)(void);
pGM130_pwrOpenDevice GM130_pwrOpenDevice;
typedef void (WINAPI *pGM130_pwrCloseDevice)(void);
pGM130_pwrCloseDevice GM130_pwrCloseDevice;
typedef int (WINAPI *pGM130_pwrGetLevel)(void);
pGM130_pwrGetLevel GM130_pwrGetLevel;
typedef int (WINAPI *pGM130_pwrGetStatus)(void);
pGM130_pwrGetStatus GM130_pwrGetStatus;

//
// Backlight manager
//
typedef BOOL (WINAPI *pGM130_blOpenDevice)(void);
pGM130_blOpenDevice GM130_blOpenDevice;
typedef void (WINAPI *pGM130_blCloseDevice)(void);
pGM130_blCloseDevice GM130_blCloseDevice;
typedef void (WINAPI *pGM130_blSetLevel)(int);
pGM130_blSetLevel GM130_blSetLevel;
typedef void (WINAPI *pGM130_blSetTimeout)(int);
pGM130_blSetTimeout GM130_blSetTimeout;
typedef void (WINAPI *pGM130_blEnAutoLevel)(BOOL);
pGM130_blEnAutoLevel GM130_blEnAutoLevel;

//
// Buzzer controls
//
typedef BOOL (WINAPI *pGM130_buzzOpenDevice)(void);
pGM130_buzzOpenDevice GM130_buzzOpenDevice;
typedef void (WINAPI *pGM130_buzzCloseDevice)(void);
pGM130_buzzCloseDevice GM130_buzzCloseDevice;
typedef void (WINAPI *pGM130_buzzSetMessageVolume)(int);
pGM130_buzzSetMessageVolume GM130_buzzSetMessageVolume;
typedef void (WINAPI *pGM130_buzzMessageMute)(BOOL);
pGM130_buzzMessageMute GM130_buzzMessageMute;

#if 0 // unused
typedef void (WINAPI *pGM130_buzzSound)(int, int, int, int);
pGM130_buzzSound GM130_buzzSound;
#endif

#if GM130TEMPERATURE
//
// Temperature sensor
//
typedef BOOL (WINAPI *pGM130_tempOpenDevice)(void);
pGM130_tempOpenDevice GM130_tempOpenDevice;
typedef void (WINAPI *pGM130_tempCloseDevice)(void);
pGM130_tempCloseDevice GM130_tempCloseDevice;
typedef int (WINAPI *pGM130_tempGetValue)(void);
pGM130_tempGetValue GM130_tempGetValue;
#endif 

bool DeviceIsGM130=false;
HMODULE hapiHandle=NULL;

const LPCTSTR HOLIB[] = {_T("GM130API.DLL"), _T("GM132API.DLL")};
//#define HODEBUG	1

bool Init_GM130(void) {

  if (DeviceIsGM130) {
	#if HODEBUG
	StartupStore(_T("... Init_GM130 already initialised%s"),NEWLINE);
	#endif
	return true;
  }

  hapiHandle = NULL;
  for(const LPCTSTR* szLib = std::begin(HOLIB); hapiHandle==(HMODULE)NULL && szLib != std::end(HOLIB); ++szLib) {
      hapiHandle=LoadLibrary(*szLib);
  }
  
  if (hapiHandle==(HMODULE)NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 ERR"));
	StartupStore(_T("... Init_GM130 LoadLibrary failed%s"),NEWLINE);
	#endif
	return false;
  }

  // Library was found, error messages ok assuming this is an Holux device

  //
  // Barometer control
  // 
  GM130_barOpenDevice = NULL;
  GM130_barOpenDevice = (pGM130_barOpenDevice) GetProcAddressA(hapiHandle, "barOpenDevice");
  if (GM130_barOpenDevice == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE barOpen"));
	StartupStore(_T("... Init_GM130 GetProcAddress barOpen failed%s"),NEWLINE);
	#endif
	goto _fail;
  } 
  GM130_barCloseDevice = NULL;
  GM130_barCloseDevice = (pGM130_barCloseDevice) GetProcAddress(hapiHandle, TEXT("barCloseDevice"));
  if (GM130_barCloseDevice == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE barClose"));
	StartupStore(_T("... Init_GM130 GetProcAddress barClose failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  GM130_barGetAltitude = NULL;
  GM130_barGetAltitude = (pGM130_barGetAltitude) GetProcAddress(hapiHandle, TEXT("barGetAltitude"));
  if (GM130_barGetAltitude == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE barGetAltitude"));
	StartupStore(_T("... Init_GM130 GetProcAddress barGetAltitude failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  GM130_barGetPressure = NULL;
  GM130_barGetPressure = (pGM130_barGetPressure) GetProcAddress(hapiHandle, TEXT("barGetPressure"));
  if (GM130_barGetPressure == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE barGetPressure"));
	StartupStore(_T("... Init_GM130 GetProcAddress barGetPressure failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  #if 0
  GM130_barCalibrate = NULL;
  GM130_barCalibrate = (pGM130_barCalibrate) GetProcAddress(hapiHandle, TEXT("barCalibrate"));
  if (GM130_barCalibrate == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE barCalibrate"));
	StartupStore(_T("... Init_GM130 GetProcAddress barCalibrate failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  #endif

  //
  // Power controls
  //
  GM130_pwrOpenDevice = NULL;
  GM130_pwrOpenDevice = (pGM130_pwrOpenDevice) GetProcAddressA(hapiHandle, "pwrOpenDevice");
  if (GM130_pwrOpenDevice == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE pwrOpen"));
	StartupStore(_T("... Init_GM130 GetProcAddress pwrOpen failed%s"),NEWLINE);
	#endif
	goto _fail;
  } 
  GM130_pwrCloseDevice = NULL;
  GM130_pwrCloseDevice = (pGM130_pwrCloseDevice) GetProcAddress(hapiHandle, TEXT("pwrCloseDevice"));
  if (GM130_pwrCloseDevice == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE pwrClose"));
	StartupStore(_T("... Init_GM130 GetProcAddress pwrClose failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  GM130_pwrGetLevel = NULL;
  GM130_pwrGetLevel = (pGM130_pwrGetLevel) GetProcAddress(hapiHandle, TEXT("pwrGetLevel"));
  if (GM130_pwrGetLevel == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE pwrGetLevel"));
	StartupStore(_T("... Init_GM130 GetProcAddress pwrGetLevel failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  GM130_pwrGetStatus = NULL;
  GM130_pwrGetStatus = (pGM130_pwrGetStatus) GetProcAddress(hapiHandle, TEXT("pwrGetStatus"));
  if (GM130_pwrGetStatus == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE pwrGetStatus"));
	StartupStore(_T("... Init_GM130 GetProcAddress pwrGetStatus failed%s"),NEWLINE);
	#endif
	goto _fail;
  }

  //
  // Backlight controls
  //
  GM130_blOpenDevice = NULL;
  GM130_blOpenDevice = (pGM130_blOpenDevice) GetProcAddressA(hapiHandle, "blOpenDevice");
  if (GM130_blOpenDevice == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE blOpen"));
	StartupStore(_T("... Init_GM130 GetProcAddress blOpen failed%s"),NEWLINE);
	#endif
	goto _fail;
  } 
  GM130_blCloseDevice = NULL;
  GM130_blCloseDevice = (pGM130_blCloseDevice) GetProcAddress(hapiHandle, TEXT("blCloseDevice"));
  if (GM130_blCloseDevice == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE blClose"));
	StartupStore(_T("... Init_GM130 GetProcAddress blClose failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  GM130_blSetLevel = NULL;
  GM130_blSetLevel = (pGM130_blSetLevel) GetProcAddress(hapiHandle, TEXT("blSetLevel"));
  if (GM130_blSetLevel == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE blSetLevel"));
	StartupStore(_T("... Init_GM130 GetProcAddress blSetLevel failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  GM130_blSetTimeout = NULL;
  GM130_blSetTimeout = (pGM130_blSetTimeout) GetProcAddress(hapiHandle, TEXT("blSetTimeout"));
  if (GM130_blSetTimeout == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE blSetTimeout"));
	StartupStore(_T("... Init_GM130 GetProcAddress blSetTimeout failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  GM130_blEnAutoLevel = NULL;
  GM130_blEnAutoLevel = (pGM130_blEnAutoLevel) GetProcAddress(hapiHandle, TEXT("blEnAutoLevel"));
  if (GM130_blEnAutoLevel == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE blEnAutoLevel"));
	StartupStore(_T("... Init_GM130 GetProcAddress blEnAutoLevel failed%s"),NEWLINE);
	#endif
	goto _fail;
  }


  //
  // Buzzer controls
  //
  GM130_buzzOpenDevice = NULL;
  GM130_buzzOpenDevice = (pGM130_buzzOpenDevice) GetProcAddressA(hapiHandle, "buzzOpenDevice");
  if (GM130_buzzOpenDevice == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE buzzOpen"));
	StartupStore(_T("... Init_GM130 GetProcAddress buzzOpen failed%s"),NEWLINE);
	#endif
	goto _fail;
  } 
  GM130_buzzCloseDevice = NULL;
  GM130_buzzCloseDevice = (pGM130_buzzCloseDevice) GetProcAddress(hapiHandle, TEXT("buzzCloseDevice"));
  if (GM130_buzzCloseDevice == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE buzzClose"));
	StartupStore(_T("... Init_GM130 GetProcAddress buzzClose failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  GM130_buzzSetMessageVolume = NULL;
  GM130_buzzSetMessageVolume = (pGM130_buzzSetMessageVolume) GetProcAddress(hapiHandle, TEXT("buzzSetMessageVolume"));
  if (GM130_buzzSetMessageVolume == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE buzzSetMessageVolume"));
	StartupStore(_T("... Init_GM130 GetProcAddress buzzSetMessageVolume failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  #if 0 // unused
  GM130_buzzSound = NULL;
  GM130_buzzSound = (pGM130_buzzSound) GetProcAddress(hapiHandle, TEXT("buzzSound"));
  if (GM130_buzzSound == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE buzzSound"));
	StartupStore(_T("... Init_GM130 GetProcAddress buzzSound failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  #endif
  GM130_buzzMessageMute = NULL;
  GM130_buzzMessageMute = (pGM130_buzzMessageMute) GetProcAddress(hapiHandle, TEXT("buzzMessageMute"));
  if (GM130_buzzMessageMute == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE buzzMessageMute"));
	StartupStore(_T("... Init_GM130 GetProcAddress buzzMessageMute failed%s"),NEWLINE);
	#endif
	goto _fail;
  }

  #if GM130TEMPERATURE
  //
  // Temperature sensor
  //
  GM130_tempOpenDevice = NULL;
  GM130_tempOpenDevice = (pGM130_tempOpenDevice) GetProcAddressA(hapiHandle, "tempOpenDevice");
  if (GM130_tempOpenDevice == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE tempOpen"));
	StartupStore(_T("... Init_GM130 GetProcAddress tempOpen failed%s"),NEWLINE);
	#endif
	goto _fail;
  } 
  GM130_tempCloseDevice = NULL;
  GM130_tempCloseDevice = (pGM130_tempCloseDevice) GetProcAddress(hapiHandle, TEXT("tempCloseDevice"));
  if (GM130_tempCloseDevice == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE tempClose"));
	StartupStore(_T("... Init_GM130 GetProcAddress tempClose failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  GM130_tempGetValue = NULL;
  GM130_tempGetValue = (pGM130_tempGetValue) GetProcAddress(hapiHandle, TEXT("tempGetValue"));
  if (GM130_tempGetValue == NULL) {
	#if HODEBUG
	DoStatusMessage(_T("INIT GM130 FAILURE tempGetValue"));
	StartupStore(_T("... Init_GM130 GetProcAddress tempGetValue failed%s"),NEWLINE);
	#endif
	goto _fail;
  }
  #endif

  #if HODEBUG
  DoStatusMessage(_T("INIT LIB GM130 OK!"));
  #endif
  StartupStore(_T(". Init Library GM130 Ok.%s"),NEWLINE);

  if (GM130_barOpenDevice() == FALSE) {
	StartupStore(_T("... Init_GM130 barOpenDevice failed%s"),NEWLINE);
	goto _fail;
  }
  if (GM130_pwrOpenDevice() == FALSE) {
	StartupStore(_T("... Init_GM130 pwrOpenDevice failed%s"),NEWLINE);
	goto _fail;
  }
  if (GM130_buzzOpenDevice() == FALSE) {
	StartupStore(_T("... Init_GM130 buzzOpenDevice failed%s"),NEWLINE);
	goto _fail;
  }
  #if GM130TEMPERATURE
  if (GM130_tempOpenDevice() == FALSE) {
	StartupStore(_T("... Init_GM130 tempOpenDevice failed%s"),NEWLINE);
	goto _fail;
  }
  #endif

  DeviceIsGM130=true;

  return true;

_fail:
  StartupStore(_T(".... Init GM130 failed!%s"),NEWLINE);

  if (GM130_barOpenDevice != NULL) GM130_barCloseDevice();
  if (GM130_pwrOpenDevice != NULL) GM130_pwrCloseDevice();
  if (GM130_buzzOpenDevice != NULL) GM130_buzzCloseDevice();
  #if GM130TEMPERATURE
  if (GM130_tempOpenDevice != NULL) GM130_tempCloseDevice();
  #endif

  FreeLibrary(hapiHandle);
  hapiHandle=NULL;
  DeviceIsGM130=false;
  return false;

}

void DeInit_GM130(void) {

  if (!DeviceIsGM130) return;
  DeviceIsGM130=false;

  GM130_barCloseDevice();
  GM130_pwrCloseDevice();
  // Backlight was already closed
  GM130_buzzCloseDevice();
  #if GM130TEMPERATURE
  GM130_tempCloseDevice();
  #endif

  FreeLibrary(hapiHandle);
  hapiHandle = NULL;

  return;

}

//
// These functions are called only if DeviceIsGM130
//

int GM130BarAltitude(void) {

  if (!DeviceIsGM130) return 0;
  return GM130_barGetAltitude();

}

float GM130BarPressure(void) {

  if (!DeviceIsGM130) return 0;
  return GM130_barGetPressure();

}

int GM130PowerLevel(void) {

  if (!DeviceIsGM130) return 0;
  return GM130_pwrGetLevel();

}

int GM130PowerStatus(void) {

  if (!DeviceIsGM130) return 0;
  int pstatus = GM130_pwrGetStatus();
  switch (pstatus) {
	case 0: 
		return  Battery::OFFLINE; // NOT_CHARGING
	case 1:
		return  Battery::ONLINE;  // CHARGING
	case 2:
		return  Battery::ONLINE;  // CHARGE_FINISHED
	case 3:
		return  Battery::UNKNOWN; // NO_BATTERY
	case 4:
		return  Battery::OFFLINE; // BAD_BATTERY
	case 5:
		return  Battery::OFFLINE; // BATTERY_DROP
	default:
		return  Battery::UNKNOWN; 
  }
}

int GM130PowerFlag(void) {

  if (!DeviceIsGM130) return 0;
  int pstatus = GM130_pwrGetStatus();
  switch (pstatus) {
	case 0: 
		return  Battery::HIGH; // NOT_CHARGING;
	case 1:
		return  Battery::CHARGING;  // CHARGING
	case 2:
		return  Battery::HIGH;  //CHARGE_FINISHED
	case 3:
		return  Battery::NO_BATTERY; // NO_BATTERY
	case 4:
		return  Battery::CRITICAL; // BAD_BATTERY
	case 5:
		return  Battery::LOW; // BATTERY_DROP
	default:
		return  Battery::UNKNOWN;
  }
}

void GM130MaxBacklight(void) {

  if (!DeviceIsGM130) return;
  if (!GM130_blOpenDevice()) return;

  GM130_blSetTimeout(0); // No timeout
  GM130_blSetLevel(9);   // Max brightness
  GM130_blEnAutoLevel(FALSE); 

  GM130_blCloseDevice();

}

// There is no real sound, only a buzzzer. A real pity on this jewel.
void GM130MaxSoundVolume(void) {

  if (!DeviceIsGM130) return;
  GM130_buzzMessageMute(FALSE);
  GM130_buzzSetMessageVolume(2); // max volume for MessageBeep buzz

}

#if 0 // unused but available
// Level 0-1-2
// Duration in ms
// Count of times
// Interval in ms between counts
void GM130PlaySound(int level, int duration, int count, int interval) {
  // Function ready to be used
  if (!DeviceIsGM130) return;
  if (level<0 || level>2) level=2;
  if (count<1|| count>100) count=1;
  GM130_buzzSound(level, duration, count, interval);
}
#endif

#if GM130TEMPERATURE
int GM130GetTemperature(void) {

  if (!DeviceIsGM130) return 0;
  return GM130_tempGetValue();

}
#endif


#endif // PNA only
