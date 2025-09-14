/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   device.h
 */

#ifndef	DEVICE_H
#define	DEVICE_H

#include "MapWindow.h"
#include "ComPort.h"
#include "Comm/PortConfig.h"
#include "BtHandler.h"
#include <vector>
#include "Util/tstring.hpp"
#include "utils/stl_utils.h"
#include "DeviceDescriptor.h"

class COMMPortItem_t {
public:
    inline COMMPortItem_t(const TCHAR* szName, const TCHAR* szLabel =_T("")) :
			_sName(szName), _sLabel(szLabel)
	{
	}

	inline COMMPortItem_t(tstring&& szName, tstring&& szLabel) :
			_sName(szName), _sLabel(szLabel)
	{
	}

#ifndef NO_BLUETOOTH
    inline COMMPortItem_t(const CBtDevice* pDev) : _sName(pDev->BTPortName()), _sLabel() {
        _sLabel = _T("BT_SPP:") + pDev->GetName();
    }

    inline COMMPortItem_t& operator=(const CBtDevice* pDev) {
        _sName = pDev->BTPortName();
        _sLabel = _T("BT_SPP:") + pDev->GetName();
        return (*this);
    }
#endif
    inline bool IsSamePort(const TCHAR* szName) const { return _sName == szName; }

    inline const TCHAR* GetName() const { return _sName.c_str(); }
    inline const TCHAR* GetLabel() const { return _sLabel.empty()?_sName.c_str():_sLabel.c_str(); }

protected:
    tstring _sName;
    tstring _sLabel;
};

typedef std::vector<COMMPortItem_t> COMMPort_t;

COMMPort_t::iterator FindCOMMPort(const TCHAR* port);

uint8_t nmea_crc(const char *text);

void devWriteNMEAString(DeviceDescriptor_t* d, const TCHAR *Text);

#ifdef ANDROID
extern Mutex COMMPort_mutex; // needed for Bluetooth LE scan
#endif
extern COMMPort_t COMMPort;

void RefreshComPortList();

void RestartCommPorts();

BOOL devInit();
void devCloseAll();
DeviceDescriptor_t* devGetDeviceOnPort(unsigned Port);
BOOL ExpectString(DeviceDescriptor_t* d, const TCHAR *token);

// return true if all device are disabled 
bool devIsDisabled();
BOOL devOpen(DeviceDescriptor_t* d);
BOOL devDirectLink(DeviceDescriptor_t* d,	BOOL bLink);

void devParseNMEA(unsigned portNum, const char *String,	NMEA_INFO	*GPS_INFO);
BOOL devParseStream(unsigned portNum, char *String,int len,	NMEA_INFO	*GPS_INFO);

BOOL devPutMacCready(double MacCready, DeviceDescriptor_t* Sender);
BOOL devRequestFlarmVersion(DeviceDescriptor_t* d);
BOOL devPutBugs(double	Bugs, DeviceDescriptor_t* Sender);
BOOL devPutBallast(double Ballast, DeviceDescriptor_t* Sender);
BOOL devPutVolume(int Volume);
BOOL devPutFreqSwap();
BOOL devPutRadioMode(int Mode);
BOOL devPutSquelch(int Volume);
BOOL devPutFreqActive(unsigned Freq, const TCHAR* StationName);
BOOL devPutFreqStandby(unsigned Freq, const TCHAR* StationName);
BOOL devLinkTimeout();
BOOL devDeclare(DeviceDescriptor_t* d, const Declaration_t *decl, unsigned errBufferLen, TCHAR errBuffer[]);
BOOL devIsLogger(DeviceDescriptor_t& d);
BOOL devIsBaroSource(const DeviceDescriptor_t& d);
BOOL devIsRadio(DeviceDescriptor_t* d);

BOOL devHeartBeat();
BOOL devPutQNH(double NewQNH);

BOOL devPutTarget(const WAYPOINT& wpt);

BOOL devSetAdvancedMode(DeviceDescriptor_t* d,	BOOL bAdvMode);
BOOL devGetAdvancedMode(DeviceDescriptor_t* d);

template<size_t idx>
BOOL devConfig() {
  static_assert(idx < std::size(DeviceList), "invalid index");
  auto& dev = DeviceList[idx];
  return dev.Config && dev.Config(&dev);
}

template<size_t idx>
void devWriteNMEA(const TCHAR *Text) {
  static_assert(idx < std::size(DeviceList), "invalid index");
  devWriteNMEAString(&DeviceList[idx], Text);
}

template<size_t idx>
void devWriteData(const TCHAR *Text) {
  static_assert(idx < std::size(DeviceList), "invalid index");
  if (Text) {
    ScopeLock Lock(CritSec_Comm);
    DeviceDescriptor_t& d = DeviceList[idx];
    if (!d.Disabled && d.Com) {
      d.Com->WriteString(Text);
    }
  }
}

void SendDataToExternalDevice(const NMEA_INFO& Basic, const DERIVED_INFO& Calculated);

TCHAR devLetter(unsigned idx);

#endif
