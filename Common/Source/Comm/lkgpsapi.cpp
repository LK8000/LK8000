#if defined(PNA) && defined(UNDER_CE)
#include <windows.h>
#include <tchar.h>
#include "lkgpsapi.h"

typedef HANDLE (WINAPI *GPSOpenDevice_t)(HANDLE hNewLocationData, HANDLE hDeviceStateChange, const WCHAR *szDeviceName, DWORD dwFlags);
typedef DWORD  (WINAPI *GPSCloseDevice_t)(HANDLE hGPSDevice);
typedef DWORD  (WINAPI *GPSGetPosition_t)(HANDLE hGPSDevice, GPS_POSITION *pGPSPosition, DWORD dwMaximumAge, DWORD dwFlags);
typedef DWORD  (WINAPI *GPSGetDeviceState_t)(GPS_DEVICE *pGPSDevice);

HINSTANCE g_hGpsApi;

GPSOpenDevice_t pGPSOpenDevice = NULL;
GPSCloseDevice_t pGPSCloseDevice = NULL;
GPSGetPosition_t pGPSGetPosition = NULL;
GPSGetDeviceState_t pGPSGetDeviceState = NULL;

void InitGpsApiFunctionPtrs(){
	g_hGpsApi = LoadLibrary(_T("GPSAPI.DLL"));
	if(g_hGpsApi) {
            pGPSOpenDevice = (GPSOpenDevice_t)::GetProcAddress(g_hGpsApi, _T("GPSOpenDevice"));
            pGPSCloseDevice = (GPSCloseDevice_t)::GetProcAddress(g_hGpsApi, _T("GPSCloseDevice"));
            pGPSGetPosition = (GPSGetPosition_t)::GetProcAddress(g_hGpsApi, _T("GPSGetPosition"));
            pGPSGetDeviceState = (GPSGetDeviceState_t)::GetProcAddress(g_hGpsApi, _T("GPSGetDeviceState"));
	}
}
void DeInitGpsApiFunctionPtrs() {
	if(g_hGpsApi) {
		FreeLibrary(g_hGpsApi);
		pGPSOpenDevice = NULL;
		pGPSCloseDevice = NULL;
		pGPSGetPosition = NULL;
		pGPSGetDeviceState = NULL;
	}
}

HANDLE GPSOpenDevice(HANDLE hNewLocationData, HANDLE hDeviceStateChange, const WCHAR *szDeviceName, DWORD dwFlags) {
	if(pGPSOpenDevice) {
		return pGPSOpenDevice(hNewLocationData, hDeviceStateChange, szDeviceName, dwFlags);
	}
	return NULL;
}

DWORD  GPSCloseDevice(HANDLE hGPSDevice) {
	if(pGPSCloseDevice) {
		return pGPSCloseDevice(hGPSDevice);
	}
	return 0UL;
}

DWORD  GPSGetPosition(HANDLE hGPSDevice, GPS_POSITION *pGPSPosition, DWORD dwMaximumAge, DWORD dwFlags) {
	if(pGPSGetPosition) {
		return pGPSGetPosition(hGPSDevice, pGPSPosition, dwMaximumAge, dwFlags);
	}
	return 0UL;
}

DWORD  GPSGetDeviceState(GPS_DEVICE *pGPSDevice){
	if(pGPSGetDeviceState) {
		return pGPSGetDeviceState(pGPSDevice);
	}
	return 0UL;
}

class CGpsApiInitializer
{
public:
	CGpsApiInitializer() { InitGpsApiFunctionPtrs(); }
	~CGpsApiInitializer() { DeInitGpsApiFunctionPtrs(); }
};

static CGpsApiInitializer	initObj;

#endif