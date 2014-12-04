/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef devCProbe_h__
#define devCProbe_h__
#include "devBase.h"
#include "utils/tstring.h"
#include "nmeaistream.h"
#include "dlgTools.h"

class WindowControl;

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
	static bool Register();
	static const TCHAR* GetName() { return TEXT("C-Probe"); }
	static BOOL Open(PDeviceDescriptor_t d, int Port);
	static BOOL Close (PDeviceDescriptor_t d);


private:
	static BOOL Install(PDeviceDescriptor_t d);

// Receive data
private:
	static BOOL ParseNMEA(DeviceDescriptor_t *d, TCHAR *String, NMEA_INFO *pINFO);

	static BOOL ParseData(tnmeastring& wiss, NMEA_INFO *pINFO );
	static BOOL ParseGyro(tnmeastring& wiss, NMEA_INFO *pINFO );
	static BOOL ParseFW(tnmeastring& wiss, NMEA_INFO *pINFO );
	static BOOL ParseName(tnmeastring& wiss, NMEA_INFO *pINFO );

// Send Command
	static BOOL GetDeviceName( PDeviceDescriptor_t d );
	static BOOL SetDeviceName( PDeviceDescriptor_t d, const std::tstring& strName );
	static BOOL GetFirmwareVersion( PDeviceDescriptor_t d );
	static BOOL SetBaroOn( PDeviceDescriptor_t d );
	static BOOL SetBaroOff( PDeviceDescriptor_t d );
	static BOOL SetZeroDeltaPressure( PDeviceDescriptor_t d );
	static BOOL SetCompassCalOn( PDeviceDescriptor_t d );
	static BOOL SetCompassCalOff( PDeviceDescriptor_t d );
	static BOOL SetCalGyro( PDeviceDescriptor_t d );

	static BOOL m_bCompassCalOn;

	static double m_abs_press;
	static double m_delta_press;

	static TCHAR m_szVersion[15];

	static Poco::Mutex* m_pCritSec_DeviceData;

	static void LockDeviceData();
	static void UnlockDeviceData();

// Config
	static BOOL Config(PDeviceDescriptor_t d);
	static void OnCloseClicked(Window* pWnd);
	static void OnCompassCalClicked(Window* pWnd);
	static void OnCalGyroClicked(Window* pWnd);
	static void OnZeroDeltaPressClicked(Window* pWnd);

	static bool OnTimer();

	static void Update();

	static CallBackTableEntry_t CallBackTable[];
	static WndForm *m_wf;
	static PDeviceDescriptor_t m_pDevice;
};
#endif // devCProbe_h__
