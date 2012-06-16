/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#ifndef devCProbe_h__
#define devCProbe_h__
#include "devBase.h"
#include "string"
#include "nmeaistream.h"
#include "dlgTools.h"


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

	static BOOL ParseData(wnmeastring& wiss, NMEA_INFO *pINFO );
	static BOOL ParseGyro(wnmeastring& wiss, NMEA_INFO *pINFO );
	static BOOL ParseFW(wnmeastring& wiss, NMEA_INFO *pINFO );
	static BOOL ParseName(wnmeastring& wiss, NMEA_INFO *pINFO );

// Send Command
	static BOOL GetDeviceName( PDeviceDescriptor_t d );
	static BOOL SetDeviceName( PDeviceDescriptor_t d, const std::wstring& strName );
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

	static CRITICAL_SECTION* m_pCritSec_DeviceData;

	static void LockDeviceData();
	static void UnlockDeviceData();

// Config
	static BOOL Config();
	static void OnCloseClicked(WindowControl * Sender);
	static void OnCompassCalClicked(WindowControl * Sender);
	static void OnCalGyroClicked(WindowControl * Sender);
	static void OnZeroDeltaPressClicked(WindowControl * Sender);

	static int OnTimer(WindowControl * Sender);

	static void Update();

	static CallBackTableEntry_t CallBackTable[];
	static WndForm *m_wf;
	static PDeviceDescriptor_t m_pDevice;
};
#endif // devCProbe_h__
