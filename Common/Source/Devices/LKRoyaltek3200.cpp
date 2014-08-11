#if defined(PNA) && defined(UNDER_CE)

#include "externs.h"

/*  ********************************************
 *
 *  Royaltek hardware support for LK8000
 *  
 *  ********************************************/

typedef struct {
  long OUT_TEMP;
  long OUT_PRESSURE;
  long OUT_ALTITUDE;
} INFO_GET, *pINFO_GET;

typedef enum {
  __OK,
  _ALTI_ERROR_1,
  _ALTI_ERROR_2,
} API_RET;

static INFO_GET bikeInfo;

//
// Baro sensor
//
typedef API_RET (WINAPI *pRoyaltek3200_GetBikeInfo)(INFO_GET *bikeInfo);
pRoyaltek3200_GetBikeInfo Royaltek3200_GetBikeInfo;

bool DeviceIsRoyaltek3200=false;
HMODULE royapiHandle=NULL;

#define ROYLIB	"BIKEINFO.DLL"
//#define ROYDEBUG	1

bool Init_Royaltek3200(void) {

  if (DeviceIsRoyaltek3200) {
	#if ROYDEBUG
	StartupStore(_T("... Init_Royaltek3200 already initialised%s"),NEWLINE);
	#endif
	return true;
  }

  bikeInfo.OUT_TEMP=0;
  bikeInfo.OUT_PRESSURE=0;
  bikeInfo.OUT_ALTITUDE=0;

  royapiHandle=LoadLibrary(TEXT(ROYLIB));
  if (royapiHandle==(HMODULE)NULL) {
	#if ROYDEBUG
	DoStatusMessage(_T("INIT ROYALTEK3200 ERR"));
	StartupStore(_T("... Init_Royaltek3200 LoadLibrary failed%s"),NEWLINE);
	#endif
	return false;
  }

  // Library was found, error messages ok assuming this is a Royaltek device

  //
  // Barometer control
  // 
  Royaltek3200_GetBikeInfo = NULL;
  Royaltek3200_GetBikeInfo = (pRoyaltek3200_GetBikeInfo) GetProcAddressA(royapiHandle, "GetBikeInfo");
  if (Royaltek3200_GetBikeInfo == NULL) {
	#if ROYDEBUG
	DoStatusMessage(_T("INIT ROYALTEK3200 FAILURE GetBikeInfo"));
	StartupStore(_T("... Init_Royaltek3200 GetProcAddress GetBikeInfo failed%s"),NEWLINE);
	#endif
	goto _fail;
  } 

  #if ROYDEBUG
  DoStatusMessage(_T("INIT LIB ROYALTEK3200 OK!"));
  #endif
  StartupStore(_T(". Init Library Royaltek3200 Ok.%s"),NEWLINE);

  DeviceIsRoyaltek3200=true;

  return true;

_fail:
  StartupStore(_T(".... Init Royaltek3200 failed!%s"),NEWLINE);

  FreeLibrary(royapiHandle);
  royapiHandle=NULL;
  DeviceIsRoyaltek3200=false;
  return false;

}

void DeInit_Royaltek3200(void) {

  if (!DeviceIsRoyaltek3200) return;
  DeviceIsRoyaltek3200=false;

  FreeLibrary(royapiHandle);
  royapiHandle = NULL;

  return;

}

//
// These functions are called only if DeviceIsRoyaltek3200
//


bool Royaltek3200_ReadBarData(void) {

  int result=Royaltek3200_GetBikeInfo(&bikeInfo);
  #if ROYDEBUG
  if (result!=__OK) {
	StartupStore(_T("...... ReadBikeInfo failed, result=%d\n"),result);
  } else {
	StartupStore(_T("...... ReadBikeInfo: Alt=%d Pressure=%d Temp=%d\n"),
		bikeInfo.OUT_ALTITUDE,
		bikeInfo.OUT_PRESSURE,
		bikeInfo.OUT_TEMP);
  }
  #endif
  return (result==__OK);
}


float Royaltek3200_GetPressure(void) {

  if (!DeviceIsRoyaltek3200) return 0;
  return (float)(bikeInfo.OUT_PRESSURE/10);
}

#if 0
int Royaltek3200_GetAltitude(void) {

  if (!DeviceIsRoyaltek3200) return 0;
  return (int)(bikeInfo.OUT_ALTITUDE);
}

float Royaltek3200_GetTemperature(void) {

  if (!DeviceIsRoyaltek3200) return 0;
  return (float)(bikeInfo.OUT_TEMP/10);
}
#endif

#endif // PNA only
