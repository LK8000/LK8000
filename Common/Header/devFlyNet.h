#ifndef _DEVFLYNET_H_
#define _DEVFLYNET_H_


BOOL FlyNetRegister(void);

BOOL FlyNetParseNMEA(PDeviceDescriptor_t d, TCHAR *String, NMEA_INFO *_INFO);

#endif // _DEVFLYNET_H_
