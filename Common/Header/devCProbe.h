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
};
#endif // devCProbe_h__
