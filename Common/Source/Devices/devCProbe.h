/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef devCProbe_h__
#define devCProbe_h__
#include "devBase.h"

#include "Util/tstring.hpp"
#include "nmeaistream.h"
#include "dlgTools.h"
#include "Devices/DeviceRegister.h"

class WindowControl;
class WndButton;

class CDevCProbe : public DevBase
{
private:
	// no ctor
	CDevCProbe() {}

	// no copy
	CDevCProbe(const CDevCProbe&) {}
	void operator=(const CDevCProbe&) {}

//Init
public:
	static constexpr
	DeviceRegister_t Register() {
		return devRegister(GetName(), &Install);
	}


	static constexpr
	const TCHAR* GetName() { 
		return TEXT("C-Probe"); 
	}
	
	static BOOL Open(DeviceDescriptor_t* d);
	static BOOL Close (DeviceDescriptor_t* d);


private:
	static void Install(DeviceDescriptor_t* d);

// Receive data
private:
	static BOOL ParseNMEA(DeviceDescriptor_t* d, const char *String, NMEA_INFO *pINFO);

	static BOOL ParseData(DeviceDescriptor_t* d, nmeastring& wiss, NMEA_INFO *pINFO );
	static BOOL ParseGyro(nmeastring& wiss, NMEA_INFO *pINFO );
	static BOOL ParseFW(nmeastring& wiss, NMEA_INFO *pINFO );
	static BOOL ParseName(nmeastring& wiss, NMEA_INFO *pINFO );

// Send Command
	static BOOL GetDeviceName(DeviceDescriptor_t* d );
	static BOOL SetDeviceName(DeviceDescriptor_t* d, const tstring& strName );
	static BOOL GetFirmwareVersion(DeviceDescriptor_t* d );
	static BOOL SetBaroOn(DeviceDescriptor_t* d );
	static BOOL SetBaroOff(DeviceDescriptor_t* d );
	static BOOL SetZeroDeltaPressure(DeviceDescriptor_t* d );
	static BOOL SetCompassCalOn(DeviceDescriptor_t* d );
	static BOOL SetCompassCalOff(DeviceDescriptor_t* d );
	static BOOL SetCalGyro(DeviceDescriptor_t* d );

	static BOOL m_bCompassCalOn;

	static double m_abs_press;
	static double m_delta_press;

	static TCHAR m_szVersion[15];

	static Mutex* m_pCritSec_DeviceData;

	static void LockDeviceData();
	static void UnlockDeviceData();

// Config
	static BOOL Config(DeviceDescriptor_t* d);
	static void OnCloseClicked(WndButton* pWnd);
	static void OnCompassCalClicked(WndButton* pWnd);
	static void OnCalGyroClicked(WndButton* pWnd);
	static void OnZeroDeltaPressClicked(WndButton* pWnd);

	static bool OnTimer(WndForm* pWnd);

	static void Update(WndForm* pWnd);

	static CallBackTableEntry_t CallBackTable[];
	static DeviceDescriptor_t* m_pDevice;
};
#endif // devCProbe_h__
